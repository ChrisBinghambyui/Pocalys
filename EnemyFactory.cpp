#include "EnemyFactory.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include "VariantData.h"
#include "raylib.h"
#include <cstdlib>

const int VARIANT_CHANCE_PERCENT = 25; // Chance an eligible spawn rolls a variant tag instead of the plain base creature

static const CreatureVariant* PickCompatibleVariant(const EnemyArchetype& archetype)
{
    if (!archetype.canHaveVariant)
    {
        return nullptr;
    }

    std::vector<const CreatureVariant*> pool;
    for (size_t i = 0; i < G_CREATURE_VARIANTS.size(); i++)
    {
        bool excluded = false;
        for (size_t e = 0; e < archetype.excludedVariantIds.size(); e++)
        {
            if (archetype.excludedVariantIds[e] == G_CREATURE_VARIANTS[i].id)
            {
                excluded = true;
                break;
            }
        }
        if (!excluded)
        {
            pool.push_back(&G_CREATURE_VARIANTS[i]);
        }
    }

    if (pool.empty())
    {
        return nullptr;
    }
    return pool[GetRandomValue(0, (int)pool.size() - 1)];
}

static int ClampMinOne(int value)
{
    if (value < 1)
    {
        return 1;
    }
    return value;
}

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
    std::string name = archetype->name;
    if (!enemy.variantId.empty())
    {
        const CreatureVariant* variant = FindVariant(enemy.variantId);
        if (variant != nullptr)
        {
            name = variant->namePrefix + name + variant->nameSuffix;
        }
    }
    return name;
}

std::string GetEnemyIntentText(const Enemy& enemy)
{
    if (enemy.isDead)
    {
        return "dead";
    }

    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr)
    {
        return "unreadable";
    }

    std::vector<std::string> livingTag = { "living" };
    FactionStance stance = GetFactionStance(archetype->factionIds, livingTag);
    if (stance == STANCE_HOSTILE)
    {
        return "hostile";
    }
    if (stance == STANCE_ALLIED)
    {
        return "friendly";
    }
    return "neutral";
}


int GetEnemyArmor(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr)
    {
        return 0;
    }
    int armor = archetype->armor;
    if (!enemy.variantId.empty())
    {
        const CreatureVariant* variant = FindVariant(enemy.variantId);
        if (variant != nullptr)
        {
            armor += variant->armorMod;
        }
    }
    return armor;
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
    float weight = archetype->weight;
    if (!enemy.variantId.empty())
    {
        const CreatureVariant* variant = FindVariant(enemy.variantId);
        if (variant != nullptr)
        {
            weight *= variant->weightMultiplier;
        }
    }
    return weight;
}

int GetEnemyDetectionRadius(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    int radius = 8;
    if (archetype != nullptr)
    {
        radius = archetype->detectionRadius;
    }
    if (!enemy.variantId.empty())
    {
        const CreatureVariant* variant = FindVariant(enemy.variantId);
        if (variant != nullptr)
        {
            radius += variant->detectionRadiusMod;
        }
    }
    if (radius < 1)
    {
        radius = 1;
    }
    return radius;
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
    enemy.homeX = x;
    enemy.homeY = y;

    enemy.str = CalculateScaledAttribute(archetype.str, floorNumber);
    enemy.end = CalculateScaledAttribute(archetype.end, floorNumber);
    enemy.agi = CalculateScaledAttribute(archetype.agi, floorNumber);
    enemy.intel = CalculateScaledAttribute(archetype.intel, floorNumber);
    enemy.wil = CalculateScaledAttribute(archetype.wil, floorNumber);
    enemy.per = CalculateScaledAttribute(archetype.per, floorNumber);
    enemy.lck = CalculateScaledAttribute(archetype.lck, floorNumber);

    const CreatureVariant* variant = nullptr;
    if (GetRandomValue(1, 100) <= VARIANT_CHANCE_PERCENT)
    {
        variant = PickCompatibleVariant(archetype);
    }
    if (variant != nullptr)
    {
        enemy.variantId = variant->id;
        enemy.str = ClampMinOne(enemy.str + variant->strMod);
        enemy.end = ClampMinOne(enemy.end + variant->endMod);
        enemy.agi = ClampMinOne(enemy.agi + variant->agiMod);
        enemy.intel = ClampMinOne(enemy.intel + variant->intelMod);
        enemy.wil = ClampMinOne(enemy.wil + variant->wilMod);
        enemy.per = ClampMinOne(enemy.per + variant->perMod);
        enemy.lck = ClampMinOne(enemy.lck + variant->lckMod);
    }

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
    enemy.inventory = GenerateEnemyLoadout(archetype, variant);
    return enemy;
}


std::vector<Item> GenerateEnemyLoadout(const EnemyArchetype& archetype, const CreatureVariant* variant)
{
    std::vector<Item> loadout;

    std::vector<int> pool = archetype.weaponTypePool;
    bool variantAddedWeapon = false;
    if (variant != nullptr)
    {
        for (size_t i = 0; i < variant->addWeaponTypeIds.size(); i++)
        {
            pool.push_back(variant->addWeaponTypeIds[i]);
            variantAddedWeapon = true;
        }
    }

    if (pool.empty())
    {
        return loadout;
    }

    int minCount = archetype.minWeaponCount;
    int maxCount = archetype.maxWeaponCount;
    if (variantAddedWeapon && maxCount < 1)
    {
        minCount = 1; // A natural fighter that just gained a variant weapon should actually carry it
        maxCount = 1;
    }

    int minMaterial = archetype.minMaterialTier;
    int maxMaterial = archetype.maxMaterialTier;
    if (variantAddedWeapon && minMaterial < 0)
    {
        minMaterial = 0; // Natural fighters have no material range of their own, give the variant weapon a real tier
        maxMaterial = 4;
    }

    int weaponCount = GetRandomValue(minCount, maxCount);
    for (int i = 0; i < weaponCount; i++)
    {
        Item weapon;
        int poolIndex = GetRandomValue(0, (int)pool.size() - 1);
        weapon.weaponTypeId = pool[poolIndex];
        weapon.materialTier = GetRandomValue(minMaterial, maxMaterial);
        weapon.condition = CONDITION_GOOD;
        loadout.push_back(weapon);
    }

    return loadout;
}
