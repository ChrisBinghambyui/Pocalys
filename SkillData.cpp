#include "SkillData.h"

// Initialize the global skill list.
// Format: { id, name, primaryAttribute, secondaryAttribute, isArmorSkill, isMagicSchool }
std::vector<SkillType> G_SKILL_TYPES = {
    // Combat Skills
    { 0, "Block", ATTRIBUTE_AGI, ATTRIBUTE_END, true, false },
    { 1, "Armorer", ATTRIBUTE_STR, ATTRIBUTE_END, false, false },
    { 2, "Medium Armor", ATTRIBUTE_END, ATTRIBUTE_NONE, true, false },
    { 3, "Heavy Armor", ATTRIBUTE_END, ATTRIBUTE_NONE, true, false },
    { 4, "Blunt Weapon", ATTRIBUTE_STR, ATTRIBUTE_NONE, false, false },
    { 5, "Long Blade", ATTRIBUTE_STR, ATTRIBUTE_AGI, false, false },
    { 6, "Axe", ATTRIBUTE_STR, ATTRIBUTE_NONE, false, false },
    { 7, "Spear", ATTRIBUTE_END, ATTRIBUTE_AGI, false, false },
    { 8, "Athletics", ATTRIBUTE_END, ATTRIBUTE_NONE, false, false },

    // Magic Skills
    { 9, "Enchant", ATTRIBUTE_INT, ATTRIBUTE_NONE, false, false },
    { 10, "Destruction", ATTRIBUTE_WIL, ATTRIBUTE_NONE, false, true },
    { 11, "Alteration", ATTRIBUTE_WIL, ATTRIBUTE_NONE, false, true },
    { 12, "Illusion", ATTRIBUTE_PER, ATTRIBUTE_NONE, false, true },
    { 13, "Conjuration", ATTRIBUTE_INT, ATTRIBUTE_NONE, false, true },
    { 14, "Mysticism", ATTRIBUTE_INT, ATTRIBUTE_NONE, false, true },
    { 15, "Restoration", ATTRIBUTE_WIL, ATTRIBUTE_NONE, false, true },
    { 16, "Alchemy", ATTRIBUTE_INT, ATTRIBUTE_NONE, false, false },

    // Stealth & Agility Skills
    { 17, "Unarmored", ATTRIBUTE_AGI, ATTRIBUTE_NONE, true, false },
    { 18, "Security", ATTRIBUTE_INT, ATTRIBUTE_AGI, false, false },
    { 19, "Sneak", ATTRIBUTE_AGI, ATTRIBUTE_NONE, false, false },
    { 20, "Acrobatics", ATTRIBUTE_AGI, ATTRIBUTE_STR, false, false },
    { 21, "Light Armor", ATTRIBUTE_AGI, ATTRIBUTE_NONE, true, false },
    { 22, "Short Blade", ATTRIBUTE_AGI, ATTRIBUTE_NONE, false, false },
    { 23, "Marksman", ATTRIBUTE_AGI, ATTRIBUTE_NONE, false, false },
    { 24, "Mercantile", ATTRIBUTE_PER, ATTRIBUTE_NONE, false, false },
    { 25, "Speechcraft", ATTRIBUTE_PER, ATTRIBUTE_NONE, false, false },
    { 26, "Hand-to-hand", ATTRIBUTE_AGI, ATTRIBUTE_STR, false, false }
};

int FindSkillIdByName(const std::string& name)
{
    for (size_t i = 0; i < G_SKILL_TYPES.size(); i++)
    {
        if (G_SKILL_TYPES[i].name == name)
        {
            return G_SKILL_TYPES[i].id;
        }
    }
    return -1;
}