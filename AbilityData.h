#pragma once
#include <string>
#include <vector>

const int HOTBAR_SIZE = 10;

enum AbilitySource
{
    ABILITY_SOURCE_WEAPON, // Granted by an equipped weapon (WeaponType.abilityIds)
    ABILITY_SOURCE_SPELL,  // Granted by a known spell
    ABILITY_SOURCE_FEAT    // Granted by an active feat
};

enum AbilityShape
{
    ABILITY_SHAPE_SELF,          // The user only
    ABILITY_SHAPE_ADJACENT,      // One neighbouring tile, chosen by aim
    ABILITY_SHAPE_LINE,          // Straight line in the aimed direction, up to range
    ABILITY_SHAPE_CONE,          // Fan in the aimed direction, up to range
    ABILITY_SHAPE_RANGED_SINGLE, // First target found along the aim, up to range
    ABILITY_SHAPE_AREA           // Radius around a chosen tile within range
};

struct AbilityDef
{
    std::string id;
    std::string name;
    std::string description;
    AbilitySource source;
    AbilityShape shape;
    int range;         // In tiles
    int staminaCost;
    int manaCost;
    int damagePercent; // 100 = a normal weapon hit
    int hitCount;      // How many separate hits are rolled
    int armorPierce = 0; // Flat armor ignored on hit, applied before the normal AR subtraction. Not read by anything yet, waiting on the ability execution engine.
};

extern std::vector<AbilityDef> G_ABILITIES;

// Single home for ability lookup by id. Null if the id is unknown.
const AbilityDef* FindAbility(const std::string& id);