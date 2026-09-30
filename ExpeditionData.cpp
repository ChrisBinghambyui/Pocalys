#include "ExpeditionData.h"
#include "raylib.h"
#include "FloorModifierData.h"

// To add an expedition: add one row. Theme tags are faction ids from G_FACTIONS.
// id, name, theme tags, theme multiplier, off-theme multiplier, minor group chance, minor tags
std::vector<ExpeditionDef> G_EXPEDITIONS = {
    { "necropolis", "The Necropolis", { "undead" }, 8.0f, 0.15f, 0.25f, {} },
    { "inferno", "The Sunken Inferno", { "demon" }, 8.0f, 0.15f, 0.25f, {} },
    { "stronghold", "The Warren Stronghold", { "goblin", "orc", "ogre", "cyclops", "roc" }, 8.0f, 0.15f, 0.25f, {} },
    { "academy", "The Drowned Academy", { "sprite", "gremlin", "golem", "mage", "naga", "djinn", "titan" }, 8.0f, 0.15f, 0.25f, {} }
};

const ExpeditionDef* FindExpedition(const std::string& id)
{
    for (size_t i = 0; i < G_EXPEDITIONS.size(); i++)
    {
        if (G_EXPEDITIONS[i].id == id)
        {
            return &G_EXPEDITIONS[i];
        }
    }
    return nullptr;
}

const ExpeditionDef* PickRandomExpedition()
{
    if (G_EXPEDITIONS.empty())
    {
        return nullptr;
    }
    int index = GetRandomValue(0, (int)G_EXPEDITIONS.size() - 1);
    return &G_EXPEDITIONS[index];
}

int GetChunkIndex(int floorNumber)
{
    if (floorNumber < 1)
    {
        return 0;
    }
    return (floorNumber - 1) / FLOORS_PER_CHUNK;
}

int GetFloorInChunk(int floorNumber)
{
    if (floorNumber < 1)
    {
        return 1;
    }
    return ((floorNumber - 1) % FLOORS_PER_CHUNK) + 1;
}

FloorParams BuildFloorParams(const ExpeditionDef* expedition, const std::vector<std::string>& modifierIds, int floorNumber)
{
    FloorParams params;
    int effectiveFloor = floorNumber + DEBUG_FLOOR_OFFSET;
    params.floorNumber = effectiveFloor;
    params.chunkIndex = GetChunkIndex(effectiveFloor);
    params.floorInChunk = GetFloorInChunk(effectiveFloor);
    params.expedition = expedition;

    for (size_t i = 0; i < modifierIds.size(); i++)
    {
        const FloorModifierDef* modifier = FindFloorModifier(modifierIds[i]);
        if (modifier == nullptr)
        {
            continue; // A typo in a contract row should not crash floor generation
        }
        params.modifiers.push_back(modifier);
        params.organizationBonus += modifier->organizationBonus;
        params.visionRadiusMod += modifier->visionRadiusMod;
        params.detectionRadiusMod += modifier->detectionRadiusMod;
        params.variantChanceBonus += modifier->variantChanceBonus;
        params.minorGroupChanceMult *= modifier->minorGroupChanceMult;
        params.dangerRating += modifier->dangerRating;
    }
    return params;
}