#pragma once
#include <string>
#include <vector>

enum SkillBonusMode
{
    SKILL_BONUS_ADD, // Adds to the skill's current level. Also what an omitted mode in the table means.
    SKILL_BONUS_SET  // Overwrites the skill's level, whatever the class gave it
};

// One line of a race's skill bonuses.
// pickCount 0 = applies to every skill in skillIds.
// pickCount N = N distinct skills are chosen at random from skillIds when a character is rolled.
struct RaceSkillRule
{
    std::vector<int> skillIds;
    int amount;
    int pickCount;
    SkillBonusMode mode;
};

// A rolled, resolved bonus. This is what a CharacterProfile stores.
struct SkillBonus
{
    int skillId;
    int amount;
    SkillBonusMode mode;
};

struct RaceData
{
    std::string id;
    std::string name;
    std::string description;
    int strMod, endMod, agiMod, intelMod, wilMod, perMod, lckMod;
    std::vector<RaceSkillRule> skillRules;
    int racialFeatId; // Feat.id in G_FEATS. Name and text live there, granted at character creation.
};

extern std::vector<RaceData> G_RACES;

// Null if the id is unknown.
const RaceData* FindRace(const std::string& id);

// Resolves a race's skill rules into concrete bonuses, rolling any "choose" lines.
std::vector<SkillBonus> RollRaceSkillBonuses(const RaceData& race);

// One display line, e.g. "+10 Destruction   |   +5 Sneak". SET bonuses are grouped ("A, B set to 5").
std::string DescribeSkillBonuses(const std::vector<SkillBonus>& bonuses);