#include <cmath>
#include <chrono>
#include <cstdio>
#include <thread>
#include <iostream>
#include <string>

#include "wooting-rgb-sdk.h"
#include <json/json.h>

#include "handler.hpp"

#define COLOR_T  222, 127, 62
#define COLOR_CT 181, 102, 0

int KeyboardHandler::bombCounter = 0;
int KeyboardHandler::fireCounter = 0;
int KeyboardHandler::scanCounter = 1;

void KeyboardHandler::wooting_handle_event(std::string strroot) {
    wooting_rgb_kbd_connected();

    Json::Value root;
    Json::Reader reader;
    reader.parse(strroot, root);

    //Team Parse
    std::string team = root["player"].get("team", "").asString();
    if (team == "CT") {
        wooting_iterative_set(0, 0, 14, 16, COLOR_CT);
    } else if (team == "T") {
        wooting_iterative_set(0, 0, 14, 16, COLOR_T);
    }

    //Prepare for Status Parse
    wooting_iterative_set(1, 2, 14, 16, 0, 0, 0);
    //Status Parse (Fire)
    int burning_value = std::stoi(root["player"]["state"].get("burning", "0").asString());
    if (burning_value > 0) {
        fireCounter = fireCounter > 5 ? 0 : fireCounter;
        wooting_iterative_set(1, 2, 14, 16, burning_value == 255 ? 170 : burning_value, 0, 0);
        wooting_rgb_array_set_single((fireCounter > 2 ? 2 : 1), (fireCounter % 3) + 14, 255, 0, 0);
        fireCounter++;
    }
    //Status Parse (Flash)
    int flash_value = std::stoi(root["player"]["state"].get("flashed", "0").asString());
    if (flash_value > 0) {
        wooting_iterative_set(1, 2, 14, 16, flash_value, flash_value, flash_value);
    }

    //Prepare for Health and Armor Parse
    wooting_iterative_set(1, 1, 1, 12, 0, 0, 0);
    //Health Parse
    int hp = std::stoi(root["player"]["state"].get("health", "100").asString());
    int hp_ratio = (int)((hp/100.0f)*13);
    wooting_iterative_set(1, 1, 1, hp_ratio, 255, 0, 0);
    //Ammo Parse
    int armor = std::stoi(root["player"]["state"].get("armor", "100").asString());
    int armor_ratio = (int)((armor/100.0f)*255.f);
    wooting_rgb_array_set_single(1, 13, 255 - armor_ratio, armor_ratio, 0);

    //Kill Parse
    int kills = std::stoi(root["player"]["state"].get("round_kills", "0").asString());
    int killsHS = std::stoi(root["player"]["state"].get("round_killhs", "0").asString());
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
    std::string bombState = root["round"].get("bomb", "").asString();
    wooting_set_arrowkeys(255, 255, 255);
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
    wooting_iterative_set(0, 0, 2, 13, 0, 0, 0);
    //Ammo Parse
    for (int i = 0; i < 9; i++) {
        if (root["player"]["weapons"]["weapon_" + std::to_string(i)].get("state", "0").asString() == "active") {
            int ammo_clip = std::stoi(root["player"]["weapons"]["weapon_" + std::to_string(i)].get("ammo_clip", "0").asString());
            int ammo_clip_max = std::stoi(root["player"]["weapons"]["weapon_" + std::to_string(i)].get("ammo_clip_max", "1").asString());
            int ammo_ratio = (int)(((double)ammo_clip)/((double)ammo_clip_max)*13);
            wooting_iterative_set(0, 0, 2, ammo_ratio, (int)(255 - (ammo_ratio/13.0f)*255.f), (int)(255 + (ammo_ratio/13.0f)*255.f), 0);
        }
    }

    //Clean with Round over Animation if needed
    if (root["round"].get("phase", "").asString() == "over" || scanCounter > 2) {
        int scanCounter_ratio = (int)((scanCounter/15.0f)*255);
        wooting_iterative_set(0, 5, 0, scanCounter-1, 0, 0, 0);
        wooting_iterative_set(0, 5, scanCounter, scanCounter, 255 - scanCounter_ratio, 0, scanCounter_ratio);
        scanCounter = scanCounter+1 > 16 ? 1 : scanCounter+1;
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