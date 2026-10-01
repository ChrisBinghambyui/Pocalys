#include "EnemyFactory.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include "VariantData.h"
#include "FloorModifierData.h"
#include "raylib.h"
#include <cstdlib>

// Chance an eligible spawn rolls a variant tag instead of the plain base creature. Climbs with depth.
const int VARIANT_BASE_CHANCE_PERCENT = 25;
const int VARIANT_CHANCE_PER_CHUNK = 10;
const int VARIANT_CHANCE_PER_FLOOR = 1; // Per floor into the chunk
const int VARIANT_CHANCE_CAP_PERCENT = 85;

static int GetVariantChancePercent(int floorNumber, int modifierBonus)
{
    int chance = VARIANT_BASE_CHANCE_PERCENT;
    chance += GetChunkIndex(floorNumber) * VARIANT_CHANCE_PER_CHUNK;
    chance += (GetFloorInChunk(floorNumber) - 1) * VARIANT_CHANCE_PER_FLOOR;
    chance += modifierBonus;
    if (chance > VARIANT_CHANCE_CAP_PERCENT)
    {
        chance = VARIANT_CHANCE_CAP_PERCENT;
    }
    return chance;
}

static bool IsVariantExcluded(const EnemyArchetype& archetype, const std::string& variantId);

// Weighted pick among the variants this archetype allows and this chunk has unlocked. Weight is
// baseWeight + weightPerChunk * chunk (from the variant row), so deeper floors lean toward elite tags.
static const CreatureVariant* PickCompatibleVariant(const EnemyArchetype& archetype, int floorNumber)
{
    if (!archetype.canHaveVariant)
    {
        return nullptr;
    }

    int chunk = GetChunkIndex(floorNumber);
    std::vector<const CreatureVariant*> pool;
    std::vector<int> weights;
    int totalWeight = 0;

    for (size_t i = 0; i < G_CREATURE_VARIANTS.size(); i++)
    {
        const CreatureVariant& candidate = G_CREATURE_VARIANTS[i];
        if (chunk < candidate.minChunk)
        {
            continue;
        }
        if (IsVariantExcluded(archetype, candidate.id))
        {
            continue;
        }
        int weight = candidate.baseWeight + (candidate.weightPerChunk * chunk);
        if (weight < 1)
        {
            weight = 1;
        }
        pool.push_back(&candidate);
        weights.push_back(weight);
        totalWeight += weight;
    }

    if (pool.empty())
    {
        return nullptr;
    }

    int roll = GetRandomValue(1, totalWeight);
    for (size_t i = 0; i < pool.size(); i++)
    {
        if (roll <= weights[i])
        {
            return pool[i];
        }
        roll -= weights[i];
    }
    return pool.back();
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

const float CHUNK_STAT_GROWTH = 0.15f; // Each 10-floor chunk adds this fraction on top of the flat per-floor growth

static float GetChunkStatMultiplier(int current_floor)
{
    return 1.0f + (CHUNK_STAT_GROWTH * (float)GetChunkIndex(current_floor));
}

int CalculateScaledAttribute(int baseValue, int current_floor)
{
    int floor_step = current_floor - 1;
    if (floor_step < 0)
    {
        floor_step = 0;
    }
    int flatValue = baseValue + (floor_step * ENEMY_ATTRIBUTE_GROWTH_PER_FLOOR);
    return (int)((float)flatValue * GetChunkStatMultiplier(current_floor) + 0.5f);
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

    if (archetype->playerStance == STANCE_HOSTILE)
    {
        return "hostile";
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
    radius += enemy.detectionBonus;
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

static bool ArchetypeMatchesTheme(const EnemyArchetype& archetype, const ExpeditionDef& expedition)
{
    for (size_t i = 0; i < archetype.factionIds.size(); i++)
    {
        for (size_t t = 0; t < expedition.themeTags.size(); t++)
        {
            if (archetype.factionIds[i] == expedition.themeTags[t])
            {
                return true;
            }
        }
    }
    return false;
}

// Rule weight after depth math, then scaled by the expedition theme. Stays at least 1 while the rule is active.
static int CalculateThemedSpawnWeight(const SpawnRule& rule, const FloorParams& params)
{
    int baseWeight = CalculateSpawnWeight(rule, params.floorNumber);
    if (baseWeight <= 0)
    {
        return 0;
    }
    if (params.expedition == nullptr)
    {
        return baseWeight;
    }

    const EnemyArchetype* archetype = FindEnemyArchetype(rule.archetype_id);
    if (archetype == nullptr)
    {
        return 0;
    }

    float multiplier = params.expedition->offThemeMultiplier;
    if (ArchetypeMatchesTheme(*archetype, *params.expedition))
    {
        multiplier = params.expedition->themeMultiplier;
    }

    // Modifier boosts stack on top of the theme scaling (an infestation still swarms a Necropolis)
    for (size_t m = 0; m < params.modifiers.size(); m++)
    {
        const FloorModifierDef* modifier = params.modifiers[m];
        if (modifier->extraSpawnTag.empty())
        {
            continue;
        }
        for (size_t f = 0; f < archetype->factionIds.size(); f++)
        {
            if (archetype->factionIds[f] == modifier->extraSpawnTag)
            {
                multiplier *= modifier->extraSpawnMult;
                break;
            }
        }
    }

    int weight = (int)((float)baseWeight * multiplier + 0.5f);
    if (weight < 1)
    {
        weight = 1;
    }
    return weight;
}

const EnemyArchetype* PickSpawnArchetype(const FloorParams& params)
{
    int totalWeight = 0;
    for (size_t i = 0; i < G_SPAWN_RULES.size(); i++)
    {
        totalWeight += CalculateThemedSpawnWeight(G_SPAWN_RULES[i], params);
    }

    if (totalWeight <= 0)
    {
        return nullptr;
    }

    int roll = GetRandomValue(1, totalWeight);
    for (size_t i = 0; i < G_SPAWN_RULES.size(); i++)
    {
        int weight = CalculateThemedSpawnWeight(G_SPAWN_RULES[i], params);
        if (roll <= weight)
        {
            return FindEnemyArchetype(G_SPAWN_RULES[i].archetype_id);
        }
        roll -= weight;
    }
    return nullptr;
}

static bool IsVariantExcluded(const EnemyArchetype& archetype, const std::string& variantId)
{
    for (size_t i = 0; i < archetype.excludedVariantIds.size(); i++)
    {
        if (archetype.excludedVariantIds[i] == variantId)
        {
            return true;
        }
    }
    return false;
}

Enemy CreateEnemy(const EnemyArchetype& archetype, int floorNumber, int x, int y, const std::string& forcedVariantId, int variantChanceBonus)
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
    if (!forcedVariantId.empty())
    {
        variant = FindVariant(forcedVariantId);
        if (variant != nullptr && IsVariantExcluded(archetype, variant->id))
        {
            variant = nullptr;
        }
    }
    else if (GetRandomValue(1, 100) <= GetVariantChancePercent(floorNumber, variantChanceBonus))
    {
        variant = PickCompatibleVariant(archetype, floorNumber);
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
