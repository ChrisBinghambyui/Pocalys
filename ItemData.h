#pragma once
#include <string>

enum EquipSlot {
    SLOT_HELMET,
    SLOT_CHEST,
    SLOT_PANTS,
    SLOT_SHOULDER_LEFT,
    SLOT_SHOULDER_RIGHT,
    SLOT_FOREARM_LEFT,
    SLOT_FOREARM_RIGHT,
    SLOT_GLOVE_LEFT,
    SLOT_GLOVE_RIGHT,
    SLOT_SHOE_LEFT,
    SLOT_SHOE_RIGHT,
    SLOT_MAIN_HAND,
    SLOT_OFF_HAND,
    SLOT_SINGLE_COUNT, // Not a real slot. Marks the end of the fixed array, used to size it.
    SLOT_AMULET, // Multi-slot category, lives in Player.equippedAmulets instead of the fixed array
    SLOT_RING,   // Multi-slot category, lives in Player.equippedRings instead of the fixed array
    SLOT_NONE    // Not equippable at all (potions, materials, quest items)
};

struct ItemArchetype
{
    std::string id;
    std::string name;
    std::string description;
    int icon_id;
    EquipSlot slot;
    int armor_rating;      // AR bonus while worn. 0 for non-armor.
    int damage_dice_count; // Weapons only. 0 if not a weapon.
    int damage_dice_sides;
    int damage_bonus;
};