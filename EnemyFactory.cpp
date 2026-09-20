#include "EnemyFactory.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include "raylib.h"
#include <cstdlib>

int CalculateSpawnWeight(const SpawnRule& rule, int current_floor)
{
    if (current_floor < rule.min_floor)
    {
        return 0;
    }
    if (current_floor > rule.max_floor)
    {
        return 0;
    }

    int floor_diff = current_floor - rule.min_floor;
    int weight = rule.base_weight + (floor_diff * rule.weight_per_floor);
    if (weight < 1)
    {
        weight = 1;
    }
    return weight;
}

int CalculateScaledHp(const EnemyArchetype& archetype, int current_floor)
{
    int floor_step = current_floor - 1;
    if (floor_step < 0)
    {
        floor_step = 0;
    }
    return archetype.base_hp + (floor_step * archetype.hp_growth);
}

int CalculateScaledDamage(const EnemyArchetype& archetype, int current_floor)
{
    int floor_step = current_floor - 1;
    if (floor_step < 0)
    {
        floor_step = 0;
    }
    return archetype.base_damage + (floor_step * archetype.damage_growth);
}


std::vector<Item> GenerateEnemyLoadout(const EnemyArchetype& archetype)
{
    std::vector<Item> loadout;

    if (archetype.weaponTypePool.empty())
    {
        return loadout;
    }

    int weaponCount = GetRandomValue(archetype.minWeaponCount, archetype.maxWeaponCount);
    for (int i = 0; i < weaponCount; i++)
    {
        Item weapon;
        int poolIndex = GetRandomValue(0, (int)archetype.weaponTypePool.size() - 1);
        weapon.weaponTypeId = archetype.weaponTypePool[poolIndex];
        weapon.materialTier = GetRandomValue(archetype.minMaterialTier, archetype.maxMaterialTier);
        weapon.condition = CONDITION_GOOD; // Actively wielded, not yet looted-and-forgotten junk
        loadout.push_back(weapon);
    }

    return loadout;
}