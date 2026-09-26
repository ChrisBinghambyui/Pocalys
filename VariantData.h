#pragma once
#include <string>
#include <vector>

struct CreatureVariant
{
    std::string id;
    std::string namePrefix; // e.g. "Master "
    std::string nameSuffix; // e.g. " Archer"
    int strMod = 0;
    int endMod = 0;
    int agiMod = 0;
    int intelMod = 0;
    int wilMod = 0;
    int perMod = 0;
    int lckMod = 0;
    int armorMod = 0;
    int detectionRadiusMod = 0;
    float weightMultiplier = 1.0f;
    std::vector<int> addWeaponTypeIds; // Extra weapon types (WeaponData.h ids) this variant can roll into the loadout
};

extern std::vector<CreatureVariant> G_CREATURE_VARIANTS;

// Null if no variant has this id.
const CreatureVariant* FindVariant(const std::string& id);
