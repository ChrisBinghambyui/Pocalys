#include "FloorModifierData.h"

// To add a modifier: add one row. Fields after dangerRating, in order:
// organizationBonus, visionRadiusMod, detectionRadiusMod, variantChanceBonus, minorGroupChanceMult, extraSpawnTag, extraSpawnMult
std::vector<FloorModifierDef> G_FLOOR_MODIFIERS = {
    { "blackout", "Blackout",
        "The lamps are dead and the dark presses close. You will hear them long before you see them.",
        3, 0, -4, 0, 0, 1.0f, "", 1.0f },

    { "vigilant", "Wary Sentries",
        "Someone has been watching the stairs. Nothing here is asleep.",
        2, 0, 0, 3, 0, 1.0f, "", 1.0f },

    { "warband", "Warband Sighted",
        "Drums, somewhere below. The locals have stopped acting like locals.",
        3, 1, 0, 0, 0, 1.0f, "", 1.0f },

    { "feud", "Feud",
        "Two powers share these halls and neither shares well. Somewhere, a second banner is flying.",
        2, 0, 0, 0, 0, 4.0f, "", 1.0f },

    { "hardened", "Hardened Ranks",
        "These are not the raw recruits. Scars, good steel, and a habit of not dying.",
        2, 0, 0, 0, 25, 1.0f, "", 1.0f },

    { "infestation", "Infestation",
        "Chitin underfoot. The walls are moving in places where walls should not.",
        1, 0, 0, 0, 0, 1.0f, "insect", 6.0f },

    { "haunted", "Haunted",
        "The cold has a direction. Whatever died here did not finish leaving.",
        2, 0, 0, 0, 0, 1.0f, "ghost", 6.0f }
};

const FloorModifierDef* FindFloorModifier(const std::string& id)
{
    for (size_t i = 0; i < G_FLOOR_MODIFIERS.size(); i++)
    {
        if (G_FLOOR_MODIFIERS[i].id == id)
        {
            return &G_FLOOR_MODIFIERS[i];
        }
    }
    return nullptr;
}