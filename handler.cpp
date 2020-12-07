#include <cmath>
#include <chrono>
#include <cstdio>
#include <thread>
#include <iostream>
#include <string>

#include "wooting-rgb-sdk.h"
#include <json/json.h>

#include "handler.hpp"

int bombIndex = 0;

void wooting_handle_event(std::string strroot) {
    wooting_rgb_kbd_connected();

    Json::Value root;
    Json::Reader reader;

    //Initial Reset
    for (int i = 0; i <= 16; i++) {
        for (int j = 0; j <= 5; j++) {
            wooting_rgb_array_set_single(j, i, 0, 0, 0);
        }   
    }

    //Team Parse
    std::string team = root["player"].get("team", "").asString();
    if (team == "CT") {
        for (int i = 14; i <= 16; i++) {
            for (int j = 0; j <= 5; j++) {
                wooting_rgb_array_set_single(j, i, 29, 138, 184);
            }
        }
    } else if (team == "T") {
        for (int i = 14; i <= 16; i++) {
            for (int j = 0; j <= 2; j++) {
                wooting_rgb_array_set_single(j, i, 222, 127, 62);
            }
        }
    }

    //Health Parse
    int hp = std::stoi(root["player"]["state"].get("health", "100").asString());
    int x = hp / 10;
    for (int i = 0; i < x; i++) {
        wooting_rgb_array_set_single(1, i + 1, 255, 0, 0);
    }

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
        wooting_set_arrowkeys(bombIndex, 0, 0);
        bombIndex = (bombIndex == 255) ? 0 : 255;  
    } else if (bombState == "defused") {
        wooting_set_arrowkeys(0, 0, 255);
    } else if (bombState == "exploded") {
        wooting_set_arrowkeys(255, 0, 0);
    }

    //Ammo Parse
    for (int i = 0; i < 9; i++) {
        if (root["player"]["weapons"]["weapon_" + std::to_string(i)].get("state", "0").asString() == "active") {
            int ammo_clip = std::stoi(root["player"]["weapons"]["weapon_" + std::to_string(i)].get("ammo_clip", "0").asString());
            int ammo_clip_max = std::stoi(root["player"]["weapons"]["weapon_" + std::to_string(i)].get("ammo_clip_max", "1").asString());
            int ammo_ratio = (int)(((double)ammo_clip)/((double)ammo_clip_max) * 12);
            for (int j = 0; j < ammo_ratio; j++) {
                if (ammo_ratio < 5) {
                    wooting_rgb_array_set_single(0, j + 2, 255, 0, 0);
                } else {
                    wooting_rgb_array_set_single(0, j + 2, 0, 255, 0);
                }
            }
            break;
        }
    }

    wooting_rgb_array_update_keyboard();
}

void wooting_exit() {
        wooting_rgb_reset_rgb();
        wooting_rgb_close();
}

void wooting_set_arrowkeys(int red, int green, int blue) {
    wooting_rgb_array_set_single(4, 15, red, green, blue);
    wooting_rgb_array_set_single(5, 14, red, green, blue);
    wooting_rgb_array_set_single(5, 15, red, green, blue);
    wooting_rgb_array_set_single(5, 16, red, green, blue);
}