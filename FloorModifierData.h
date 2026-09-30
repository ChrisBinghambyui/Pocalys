#pragma once
#include <string>
#include <vector>

// Testing aid until the contact board exists. Put a modifier id here (for example "blackout") and every
// new run starts with it active. Empty means none.
const std::string DEBUG_FORCED_MODIFIER_ID = "";

// One rule change for the floors of an expedition. Pure data: contracts (phase 7) list modifier ids,
// and BuildFloorParams sums them into FloorParams. To add a modifier that uses existing levers, add one row.
struct FloorModifierDef
{
    std::string id;
    std::string name;
    std::string description;
    int dangerRating;                  // Feeds contract reward scaling later. Shown on the HUD for now.
    int organizationBonus = 0;         // Added to every group's rolled organization level, kept inside 0 to 3
    int visionRadiusMod = 0;           // Added to the player's fog-of-war reveal radius
    int detectionRadiusMod = 0;        // Added to every enemy's detection radius, baked in at spawn
    int variantChanceBonus = 0;        // Percent points added to the chance a spawn rolls a variant tag
    float minorGroupChanceMult = 1.0f; // Multiplies the expedition's hidden-minority chance per floor
    std::string extraSpawnTag = "";    // Archetypes carrying this faction id get their spawn weight multiplied...
    float extraSpawnMult = 1.0f;       // ...by this
};

extern std::vector<FloorModifierDef> G_FLOOR_MODIFIERS;

// Null if no modifier has this id.
const FloorModifierDef* FindFloorModifier(const std::string& id);