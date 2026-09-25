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

const EnemyArchetype* FindEnemyArchetype(const std::string& id)
{
    for (size_t i = 0; i < G_ENEMY_ARCHETYPES.size(); i++)
    {
        if (G_ENEMY_ARCHETYPES[i].id == id)
        {
            return &G_ENEMY_ARCHETYPES[i];
        }
    }
    return nullptr;
}

std::string GetEnemyDisplayName(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr)
    {
        return "creature";
    }
    return archetype->name;
}


int GetEnemyArmor(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr)
    {
        return 0;
    }
    return archetype->armor;
}

int GetEnemySpeed(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr)
    {
        return 100;
    }
    return archetype->speed;
}

const EnemyArchetype* PickSpawnArchetype(int current_floor)
{
    int totalWeight = 0;
    for (size_t i = 0; i < G_SPAWN_RULES.size(); i++)
    {
        totalWeight += CalculateSpawnWeight(G_SPAWN_RULES[i], current_floor);
    }

    if (totalWeight <= 0)
    {
        return nullptr;
    }

    int roll = GetRandomValue(1, totalWeight);
    for (size_t i = 0; i < G_SPAWN_RULES.size(); i++)
    {
        int weight = CalculateSpawnWeight(G_SPAWN_RULES[i], current_floor);
        if (roll <= weight)
        {
            return FindEnemyArchetype(G_SPAWN_RULES[i].archetype_id);
        }
        roll -= weight;
    }
    return nullptr;
}

Enemy CreateEnemy(const EnemyArchetype& archetype, int floorNumber, int x, int y)
{
    Enemy enemy = Enemy(); // Value-initialized: attributes, stamina, mana start at 0 until archetypes carry that data
    enemy.archetypeId = archetype.id;
    enemy.spawnFloor = floorNumber;
    enemy.x = x;
    enemy.y = y;
    enemy.maxHp = CalculateScaledHp(archetype, floorNumber);
    enemy.hp = enemy.maxHp;
    enemy.symbol = archetype.glyph;
    enemy.color = archetype.color;
    enemy.isDead = false;
    enemy.inventory = GenerateEnemyLoadout(archetype);
    return enemy;
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