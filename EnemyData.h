#pragma once
#include "raylib.h"
#include <string>
#include <vector>

struct EnemyArchetype
{
    std::string id;
    std::string name;
    std::string description;
    int icon_id; // Raylib icon index (e.g. ICON_SKULL, ICON_SWORD)
    int str;
    int end;
    int agi;
    int intel;
    int wil;
    int per;
    int lck;

    int armor;
    float weight; // Creature weight, for drag/reanimate/grapple/throw math (not yet used)

    std::vector<int> weaponTypePool; // WeaponType ids (WeaponData.h) this archetype can carry. Empty = natural weapons only, base_damage applies.
    int minMaterialTier;             // Index into G_MATERIAL_TIERS
    int maxMaterialTier;
    int minWeaponCount;              // How many rolled weapons this archetype spawns with
    int maxWeaponCount;


    std::vector<std::string> factionIds; // Ordered highest-priority first. See FactionData.h.

    char glyph = '?';    // Map symbol, copied onto Enemy.symbol at spawn
    Color color = WHITE; // Map color, copied onto Enemy.color at spawn
    int speed = 100; // Energy gained per player action. 100 = acts once per player turn, 200 = twice, 50 = every other turn.
};

struct SpawnRule
{
    std::string archetype_id;
    int min_floor;
    int max_floor;
    int base_weight;
    int weight_per_floor;
};


extern std::vector<EnemyArchetype> G_ENEMY_ARCHETYPES;
extern std::vector<SpawnRule> G_SPAWN_RULES;