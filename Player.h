#pragma once
#include <vector>
#include <string>
#include "Item.h"
#include "ItemData.h"

struct Player {
    std::string name;
    std::string className;
    std::string birthsign;

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

    std::vector<Item> inventory;               // Carried, unequipped items
    Item equippedSlots[SLOT_SINGLE_COUNT];      // Indexed by EquipSlot
    std::vector<Item> equippedAmulets;          // Cap 2
    std::vector<Item> equippedRings;            // Cap 10
};