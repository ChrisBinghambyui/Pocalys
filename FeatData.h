#pragma once
#include <string>
#include <vector>
#include "SkillData.h"

// Forward declaration of Player to use in prerequisite checks
struct Player;

enum FeatCategory {
    FEAT_MARTIAL,
    FEAT_DEFENSE,
    FEAT_UTILITY,
    FEAT_MAGIC,
    FEAT_STEALTH_SOCIAL,
    BOON_MARTIAL,
    BOON_ARCANE,
    BOON_STEALTH_SOCIAL,
    FEAT_RACIAL,
    BOON_DEFENSIVE,
    BOON_EXPLORATION_CRAFT,
    BOON_UNIVERSAL
};

struct PrereqSkill {
    int skillId;      // ID from G_SKILL_TYPES (-1 if any skill applies)
    int minLevel;
};

struct PrereqAttribute {
    Attribute attr;   // Enum from SkillData.h
    int minVal;
};

struct Feat {
    int id;
    std::string name;
    FeatCategory category;
    std::string description;
    bool isBoon;
    bool isRepeatable;

    // Prerequisites
    std::vector<PrereqAttribute> attrPrereqs;
    std::vector<PrereqSkill> skillPrereqs;
    std::vector<int> requiredFeatIds; // Feat or Boon IDs needed before taking this
    std::vector<std::string> requiredRaceIds; // Race ids (G_RACES) allowed to take this. Empty = any race.
};

extern std::vector<Feat> G_FEATS;

// Null if no feat has this id. Ids have gaps, so this is a lookup, not an index.
const Feat* FindFeat(int featId);

// Global helper to check if a player meets the requirements for a Feat or Boon
bool canPlayerTakeFeat(const Player& player, int featId);