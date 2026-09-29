#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// LEGACY. Only picks a fallback brain in BrainData.cpp. Delete once every archetype row sets brainId.
enum AIBrain
{
    BRAIN_MINDLESS,
    BRAIN_PREDATOR,
    BRAIN_SKITTISH,
    BRAIN_TERRITORIAL,
    BRAIN_CASTER
};

// Where a squad member stands relative to the target. See Squad.cpp for the formation math.
enum EnemyRole
{
    ROLE_FRONTLINE,  // Shield-wall arc at melee range, facing the target
    ROLE_SKIRMISHER, // Same range, but approaches from a wide flank angle instead of head-on
    ROLE_BACKLINE    // Held well behind the frontline, out of melee range
};

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
    AIBrain brain = BRAIN_MINDLESS;
    int detectionRadius = 8; // Tiles of line-of-sight range. Lower for mindless/small creatures, higher for keen hunters.
    bool canHaveVariant = true; // False for swarms/masses where a tag like "Archer" wouldn't make sense
    std::vector<std::string> excludedVariantIds; // Variant ids this archetype can never roll (e.g. no hands for a bow)
    std::string brainId = ""; // Key into G_BRAINS (BrainData.h). Empty falls back to the legacy brain enum.
    EnemyRole role = ROLE_FRONTLINE;
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

// Patches role and brainId onto specific archetypes by id, after G_ENEMY_ARCHETYPES exists.
// Same pattern as ResolveWeaponSkillIds(). Call once at startup.
void ResolveEnemyArchetypeDefaults();
