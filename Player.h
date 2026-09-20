#pragma once
#include <vector>
#include "Item.h"
#include "ItemData.h" // For EquipSlot

// NEW: Everything that makes up our player character
struct Player {
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
    Item equippedSlots[SLOT_SINGLE_COUNT];      // Indexed by EquipSlot, empty archetypeId = nothing equipped
    std::vector<Item> equippedAmulets;          // Cap 2, enforced at equip time
    std::vector<Item> equippedRings;            // Cap 10, enforced at equip time
};