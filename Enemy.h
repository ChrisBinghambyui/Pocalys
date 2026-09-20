#pragma once
#include "raylib.h"
#include <vector>
#include "Item.h"
#include <string>

struct Enemy {
    int x;
    int y;
    int hp;
    int maxHp;
    int stamina;
    int maxStamina;
    int mana;
    int maxMana;
    int str;
    int end;
    int agi;
    int intel;
    int wil;
    int per;
    int lck;
    char symbol; // What character represents this enemy (e.g., 'g' for goblin)
    Color color; // What color to draw them
    bool isDead; // Corpses stay on the map but stop blocking movement/attacks
    std::vector<Item> inventory; // Loot available once this corpse is opened as a container
};