#pragma once
#include <string>

enum MaterialAffinity {
    AFFINITY_LIGHT,  // Bonus applies to Light/Ranged weapon types, armor weighs less
    AFFINITY_HEAVY,  // Bonus applies to Medium/Heavy weapon types, armor rates higher
    AFFINITY_NONE    // Sundered only. Transcends the pairing, bonus applies everywhere.
};

struct MaterialTier
{
    int tier;         // 0 through 20, overall index
    int pairIndex;    // 0 through 9, shared by both halves of a pair. -1 for Sundered.
    MaterialAffinity affinity;
    std::string name;
    std::string description;
    int damageBonus;        // Shared power level across the pair
    int armorRatingBonus;   // Shared power level across the pair
    float weightModifier;   // Multiplies base item weight. <1.0 light, >1.0 heavy
    int degradationRate;    // % chance per landed/taken hit to drop one condition tier. 0 = never degrades.
};

#include <vector>

extern std::vector<MaterialTier> G_MATERIAL_TIERS;

enum ConditionTier {
    CONDITION_PRISTINE,
    CONDITION_GOOD,
    CONDITION_WORN,
    CONDITION_DAMAGED,
    CONDITION_BROKEN
};