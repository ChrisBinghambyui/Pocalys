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

const int ENEMY_ATTRIBUTE_GROWTH_PER_FLOOR = 2;

int CalculateScaledAttribute(int baseValue, int current_floor)
{
    int floor_step = current_floor - 1;
    if (floor_step < 0)
    {
        floor_step = 0;
    }
    return baseValue + (floor_step * ENEMY_ATTRIBUTE_GROWTH_PER_FLOOR);
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

float GetEnemyWeight(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr)
    {
        return 0.0f;
    }
    return archetype->weight;
}

FactionStance GetStance(const Enemy& actor, const Enemy& target)
{
    const EnemyArchetype* actorArchetype = FindEnemyArchetype(actor.archetypeId);
    const EnemyArchetype* targetArchetype = FindEnemyArchetype(target.archetypeId);
    if (actorArchetype == nullptr || targetArchetype == nullptr)
    {
        return STANCE_NEUTRAL;
    }
    return GetFactionStance(actorArchetype->factionIds, targetArchetype->factionIds);
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
    Enemy enemy = Enemy();
    enemy.archetypeId = archetype.id;
    enemy.spawnFloor = floorNumber;
    enemy.x = x;
    enemy.y = y;

    enemy.str = CalculateScaledAttribute(archetype.str, floorNumber);
    enemy.end = CalculateScaledAttribute(archetype.end, floorNumber);
    enemy.agi = CalculateScaledAttribute(archetype.agi, floorNumber);
    enemy.intel = CalculateScaledAttribute(archetype.intel, floorNumber);
    enemy.wil = CalculateScaledAttribute(archetype.wil, floorNumber);
    enemy.per = CalculateScaledAttribute(archetype.per, floorNumber);
    enemy.lck = CalculateScaledAttribute(archetype.lck, floorNumber);

    // Same derivation as CharacterGenerator.cpp for the player. SPD isn't modeled in this codebase
    // (see RaceData.cpp), so a flat 40 stands in for it in the stamina formula, matching the player's.
    enemy.maxHp = enemy.end / 5;
    if (enemy.maxHp < 1)
    {
        enemy.maxHp = 1;
    }
    enemy.hp = enemy.maxHp;

    enemy.maxMana = enemy.intel / 5;
    enemy.mana = enemy.maxMana;

    enemy.maxStamina = (enemy.end + enemy.str + enemy.agi + 40) / 10;
    enemy.stamina = enemy.maxStamina;

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