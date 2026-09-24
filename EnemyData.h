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
    int base_hp;
    int hp_growth;
    int base_damage;
    int damage_growth;
    int armor;

    std::vector<int> weaponTypePool; // WeaponType ids (WeaponData.h) this archetype can carry. Empty = natural weapons only, base_damage applies.
    int minMaterialTier;             // Index into G_MATERIAL_TIERS
    int maxMaterialTier;
    int minWeaponCount;              // How many rolled weapons this archetype spawns with
    int maxWeaponCount;

    char glyph = '?';    // Map symbol, copied onto Enemy.symbol at spawn
    Color color = WHITE; // Map color, copied onto Enemy.color at spawn
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