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

extern std::vector<SkillType> G_SKILL_TYPES;