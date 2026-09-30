#pragma once
#include <string>
#include <vector>

const int FLOORS_PER_CHUNK = 10;
const int DEBUG_FLOOR_OFFSET = 0; // Testing aid. Set to 10 to make the first floor generate as floor 11 (chunk 1). Keep 0 for real builds.

struct FloorModifierDef; // FloorModifierData.h

struct ExpeditionDef
{
    std::string id;
    std::string name;
    std::vector<std::string> themeTags; // Matched against ANY entry in an archetype's factionIds, not just the first
    float themeMultiplier;              // Spawn weight multiplier for on-theme archetypes
    float offThemeMultiplier;           // Spawn weight multiplier for everything else. Below 1 keeps strays rare.
    float minorGroupChance;             // Phase 2: chance per floor of a hidden minority group
    std::vector<std::string> minorTags; // Phase 2: empty = anything hostile to the theme
};

extern std::vector<ExpeditionDef> G_EXPEDITIONS;

// Null if no expedition has this id.
const ExpeditionDef* FindExpedition(const std::string& id);

// Random expedition for now. The contact board picks one on purpose later. Null if the table is empty.
const ExpeditionDef* PickRandomExpedition();

// Everything the floor generator needs to know about the floor it is building. Later phases add
// contract modifiers here so GenerateFloor never grows another argument.
struct FloorParams
{
    int floorNumber;              // 1-based, absolute. Still drives spawn rules and attribute scaling.
    int chunkIndex;               // 0-based. Floors 1-10 are chunk 0, 11-20 are chunk 1.
    int floorInChunk;             // 1 to FLOORS_PER_CHUNK
    const ExpeditionDef* expedition; // Null means no theme, every rule at its plain weight

    // Active modifiers, looked up by id. Kept as a list for effects that cannot be summed (extra spawn tags).
    std::vector<const FloorModifierDef*> modifiers;

    // Summed from the list by BuildFloorParams, so consumers read one number instead of looping.
    int organizationBonus = 0;
    int visionRadiusMod = 0;
    int detectionRadiusMod = 0;
    int variantChanceBonus = 0;
    float minorGroupChanceMult = 1.0f;
    int dangerRating = 0;
};

int GetChunkIndex(int floorNumber);
int GetFloorInChunk(int floorNumber);
FloorParams BuildFloorParams(const ExpeditionDef* expedition, const std::vector<std::string>& modifierIds, int floorNumber);