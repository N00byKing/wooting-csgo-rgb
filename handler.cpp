#include <cmath>
#include <chrono>
#include <cstdio>
#include <thread>
#include <iostream>
#include <string>

#include "wooting-rgb-sdk.h"

#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

#include "handler.hpp"

#define COLOR_T  222, 127, 62
#define COLOR_CT 0, 0, 255

#define COLOR_WHITE 255, 255, 255
#define COLOR_BLACK 0, 0, 0
 
int KeyboardHandler::bombCounter = 0;
int KeyboardHandler::fireCounter = 0;
int KeyboardHandler::backCounter = 1;
int KeyboardHandler::scanCounter = 1;

void KeyboardHandler::wooting_handle_event(std::string strroot) {
    wooting_rgb_kbd_connected();

    rapidjson::Document root;
    root.Parse(strroot.c_str());

    //Clean with Round over Animation if needed
    if (scanCounter > 2 || strcmp(rapidjson::Pointer("/round/phase").GetWithDefault(root, "").GetString(),"over") == 0) {
        int scanCounter_ratio = (int)(((scanCounter >= 16 ? 16 : scanCounter )/16.0f)*255);

        wooting_iterative_set(0, 5, 0, scanCounter-5, 0, 0, 0); //Clear Area before Trail

        //Trail
        wooting_iterative_set(0, 5, scanCounter-2, scanCounter-1, (255 - scanCounter_ratio)*0.5f, 0, scanCounter_ratio*0.5f);
        wooting_iterative_set(0, 5, scanCounter-4, scanCounter-3, (255 - scanCounter_ratio)*0.1f, 0, scanCounter_ratio*0.1f);

        //Sweep Head
        wooting_iterative_set(0, 5, scanCounter, scanCounter, 255 - scanCounter_ratio, 0, scanCounter_ratio);

        scanCounter = scanCounter >= 21 ? 1 : scanCounter+1;

        //Finalize changes
        wooting_rgb_array_update_keyboard();
        return;
    }

    //"Background"
    if (backCounter <= 15) {
        wooting_iterative_set(2, 5, 1, 13, 255 - (backCounter*17), (backCounter*17), 0);
        backCounter++;
    } else if (backCounter > 15 && backCounter <= 30) {
        wooting_iterative_set(2, 5, 1, 13, 0, 255 - ((backCounter % 16)*17), ((backCounter % 16)*17));
        backCounter++;
    } else if (backCounter > 30 && backCounter <= 45) {
        wooting_iterative_set(2, 5, 1, 13, ((backCounter % 31)*17), 0, 255 - ((backCounter % 31)*17));
        backCounter++;
    } else {
        backCounter = 0;
    }
    


    //Team Parse
    std::string team = rapidjson::Pointer("/player/team").GetWithDefault(root, "").GetString();
    if (team == "CT") {
        wooting_iterative_set(0, 0, 14, 16, COLOR_CT);
    } else if (team == "T") {
        wooting_iterative_set(0, 0, 14, 16, COLOR_T);
    }

    //Prepare for Status Parse
    wooting_iterative_set(1, 2, 14, 16, COLOR_BLACK);
    wooting_iterative_set(1, 1, 1, 12, COLOR_BLACK);
    //Status Parse (Fire)
    int burning_value = rapidjson::Pointer("/player/state/burning").GetWithDefault(root, 0).GetInt();
    if (burning_value > 0) {
        fireCounter = fireCounter > 5 ? 0 : fireCounter;
        wooting_iterative_set(1, 2, 14, 16, burning_value == 255 ? 170 : burning_value, 0, 0);
        wooting_rgb_array_set_single((fireCounter > 2 ? 2 : 1), (fireCounter % 3) + 14, 255, 0, 0);
        fireCounter++;
    }
    //Status Parse (Flash)
    int flash_value = rapidjson::Pointer("/player/state/flashed").GetWithDefault(root, 0).GetInt();
    if (flash_value > 0) {
        wooting_iterative_set(1, 2, 14, 16, flash_value, flash_value, flash_value);
    }
    //Status Parse (Health)
    int hp = rapidjson::Pointer("/player/state/health").GetWithDefault(root, 100).GetInt();
    int hp_ratio = (int)((hp/100.0f)*13);
    wooting_iterative_set(1, 1, 1, hp_ratio, 255, 0, 0);
    //Status Parse (Armor)
    int armor = rapidjson::Pointer("/player/state/armor").GetWithDefault(root, 0).GetInt();
    int armor_ratio = (int)((armor/100.0f)*255.f);
    wooting_rgb_array_set_single(1, 13, 255 - armor_ratio, armor_ratio, 0);

    //Prepare for Kill Parse
    wooting_iterative_set(0, 5, 0, 0, COLOR_BLACK);
    //Kill Parse
    int kills = rapidjson::Pointer("/player/state/round_kills").GetWithDefault(root, 0).GetInt();
    int killsHS = rapidjson::Pointer("/player/state/round_killhs").GetWithDefault(root, 0).GetInt();
    if (kills > 5) {
        kills = 5;
    }
    if (killsHS > 5) {
        killsHS = 5;
    }
    while (kills > 0) {
        if (killsHS > 0) {
            wooting_rgb_array_set_single(kills, 0, 0, 255, 0);
            killsHS--;
        } else {
            wooting_rgb_array_set_single(kills, 0, 0, 0, 255);
        }
        kills--;
    }

    //Bomb Parse
    std::string bombState = rapidjson::Pointer("/round/bomb").GetWithDefault(root, "").GetString();
    wooting_set_arrowkeys(COLOR_WHITE);
    if (bombState == "planted") {
        switch (bombCounter) {
            case 0:
                bombCounter++;
                wooting_rgb_array_set_single(4, 15, 255, 0, 0);
                break;
            case 1:
                bombCounter++;
                wooting_rgb_array_set_single(5, 14, 255, 0, 0);
                break;
            case 2:
                bombCounter++;
                wooting_rgb_array_set_single(5, 15, 255, 0, 0);
                break;
            case 3:
                bombCounter = 0;
                wooting_rgb_array_set_single(5, 16, 255, 0, 0);
                break;
        }
    } else if (bombState == "defused") {
        wooting_set_arrowkeys(0, 0, 255);
    } else if (bombState == "exploded") {
        wooting_set_arrowkeys(255, 0, 0);
    }

    //Prepare for Ammo Parse
    wooting_iterative_set(0, 0, 2, 13, COLOR_BLACK);
    //Ammo Parse
    for (int i = 0; i < 9; i++) {
        if (strcmp(rapidjson::Pointer(("/player/weapons/weapon_" + std::to_string(i) + "/state").c_str()).GetWithDefault(root, "").GetString(), "active") == 0) {
            int ammo_clip = rapidjson::Pointer(("/player/weapons/weapon_" + std::to_string(i) + "/ammo_clip").c_str()).GetWithDefault(root, 0).GetInt();
            int ammo_clip_max = rapidjson::Pointer(("/player/weapons/weapon_" + std::to_string(i) + "/ammo_clip_max").c_str()).GetWithDefault(root, 0).GetInt();
            int ammo_ratio = (int)(((double)ammo_clip)/((double)ammo_clip_max)*13);
            wooting_iterative_set(0, 0, 2, ammo_ratio, (int)(255 - (ammo_ratio/13.0f)*255.f), (int)(255 + (ammo_ratio/13.0f)*255.f), 0);
        }
    }

    //Finalize changes
    wooting_rgb_array_update_keyboard();
}

void KeyboardHandler::wooting_exit() {
        wooting_rgb_reset_rgb();
        wooting_rgb_close();
}

void KeyboardHandler::wooting_set_arrowkeys(int red, int green, int blue) {
    wooting_rgb_array_set_single(4, 15, red, green, blue);
    wooting_rgb_array_set_single(5, 14, red, green, blue);
    wooting_rgb_array_set_single(5, 15, red, green, blue);
    wooting_rgb_array_set_single(5, 16, red, green, blue);
}

void KeyboardHandler::wooting_iterative_set(int i1, int i2, int j1, int j2, int red, int green, int blue) {
    for (int i = i1; i <= i2; i++) {
        for (int j = j1; j <= j2; j++) {
            wooting_rgb_array_set_single(i, j, red, green, blue);
        }
    }
}