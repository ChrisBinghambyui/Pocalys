#pragma once
#include <string>
#include <vector>
#include "Enemy.h"

// One line of a brain: which behavior, and how much this creature likes it.
struct BrainEntry
{
    std::string behaviorId; // Key into G_BEHAVIORS
    int baseWeight;         // Personality. Multiplied by the behavior's context score at pick time.
};

struct BrainDef
{
    std::string id;
    std::vector<BrainEntry> entries;
    std::vector<std::string> reflexIds; // Behaviors that override the roll whenever their score is above 0. Checked every frame.
};

extern std::vector<BrainDef> G_BRAINS;

// Null if no brain has this id.
const BrainDef* FindBrain(const std::string& id);

// The brain for this enemy: its archetype's brainId, or a legacy AIBrain fallback if brainId is empty.
const BrainDef* GetBrainForEnemy(const Enemy& enemy);