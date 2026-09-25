#pragma once
#include <string>
#include "MaterialData.h" // For ConditionTier

struct Item {
    std::string archetypeId = ""; // Non-procedural items (potions, quest items). Empty if this is a generated weapon/armor piece instead.
    int weaponTypeId = -1;        // Index into G_WEAPON_TYPES. -1 if not a generated weapon.
    int materialTier = -1;        // Index into G_MATERIAL_TIERS. -1 if not a generated weapon/armor piece.
    ConditionTier condition = CONDITION_PRISTINE;
    int quantity = 1;             // For future stackables (potions, arrows)
    int ammoTypeId = -1;           // Index into G_ARROW_TYPES. -1 if this item is not ammunition.

    bool IsEmpty() const {
        return archetypeId.empty() && weaponTypeId == -1 && ammoTypeId == -1;
    }
};