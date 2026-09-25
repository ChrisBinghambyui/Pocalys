#include "AbilityData.h"

// id, name, description, source, shape, range, stamina cost, mana cost, damage %, hit count
std::vector<AbilityDef> G_ABILITIES = {
    { "power_strike", "Power Strike", "A committed blow with everything behind it.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_ADJACENT, 1, 2, 0, 150, 1 },
    { "lunge", "Lunge", "Close the gap and drive the point home.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_LINE, 2, 2, 0, 100, 1 },
    { "flurry", "Flurry", "Three fast cuts, none of them hard.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_ADJACENT, 1, 2, 0, 45, 3 },
    { "cleave", "Cleave", "A wide swing that catches everything in front of you.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_CONE, 1, 3, 0, 80, 1 },
    { "impale", "Impale", "Run the blade through one enemy and into the one behind.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_LINE, 2, 3, 0, 110, 1 },
    { "crushing_blow", "Crushing Blow", "A slow, ugly overhead strike that dents armor.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_ADJACENT, 1, 3, 0, 180, 1 },
    { "quick_shot", "Quick Shot", "A fast, unaimed arrow loosed at whatever is closest.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_RANGED_SINGLE, 10, 1, 0, 90, 1 },
    { "aimed_shot", "Aimed Shot", "A held breath and a steady point, ignores 2 points of armor.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_RANGED_SINGLE, 10, 3, 0, 120, 1, 2 },
    { "rapid_volley", "Rapid Volley", "Three arrows loosed in quick succession, none as hard as a full draw.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_RANGED_SINGLE, 8, 4, 0, 40, 3 },
    { "piercing_shot", "Piercing Shot", "A heavy draw meant to punch through one target and find the one behind it.", ABILITY_SOURCE_WEAPON, ABILITY_SHAPE_LINE, 10, 3, 0, 110, 1 }
};

const AbilityDef* FindAbility(const std::string& id)
{
    for (size_t i = 0; i < G_ABILITIES.size(); i++)
    {
        if (G_ABILITIES[i].id == id)
        {
            return &G_ABILITIES[i];
        }
    }
    return nullptr;
}