#pragma once
#include <string>
#include <vector>

enum Attribute
{
    ATTRIBUTE_NONE,
    ATTRIBUTE_STR,
    ATTRIBUTE_END,
    ATTRIBUTE_AGI,
    ATTRIBUTE_INT,
    ATTRIBUTE_WIL,
    ATTRIBUTE_PER,
    ATTRIBUTE_LCK
};

struct SkillType
{
    int id;
    std::string name;
    Attribute primaryAttribute;
    Attribute secondaryAttribute;
    bool isArmorSkill;
    bool isMagicSchool;
};

// Must match the order of G_SKILL_TYPES exactly (id equals index). Append only.
enum SkillId
{
    SKILL_BLOCK,
    SKILL_ARMORER,
    SKILL_MEDIUM_ARMOR,
    SKILL_HEAVY_ARMOR,
    SKILL_BLUNT,
    SKILL_LONG_BLADE,
    SKILL_AXE,
    SKILL_SPEAR,
    SKILL_ATHLETICS,
    SKILL_ENCHANT,
    SKILL_DESTRUCTION,
    SKILL_ALTERATION,
    SKILL_ILLUSION,
    SKILL_CONJURATION,
    SKILL_MYSTICISM,
    SKILL_RESTORATION,
    SKILL_ALCHEMY,
    SKILL_UNARMORED,
    SKILL_SECURITY,
    SKILL_SNEAK,
    SKILL_ACROBATICS,
    SKILL_LIGHT_ARMOR,
    SKILL_SHORT_BLADE,
    SKILL_MARKSMAN,
    SKILL_MERCANTILE,
    SKILL_SPEECHCRAFT,
    SKILL_HAND_TO_HAND
};

extern std::vector<SkillType> G_SKILL_TYPES;

// Skill id for an exact name match against G_SKILL_TYPES. -1 if no skill has that name.
int FindSkillIdByName(const std::string& name);