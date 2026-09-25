#include "WeaponData.h"
#include "SkillData.h"
#include <vector>

std::vector<WeaponType> G_WEAPON_TYPES = {
    // id, name, category, skill, pri dice, pri type, sec dice, sec type, 2h, versatile, reach, thrown
        { 0,  "Dagger",       WEAPON_LIGHT,  "Short Blade",  1, 4,  "Piercing", 1, 4,  "Slashing", false, false, false, true,  { "lunge", "flurry", "power_strike" } },
    { 1,  "Shortsword",   WEAPON_LIGHT,  "Short Blade",  1, 6,  "Slashing", 1, 4,  "Piercing", false, false, false, false },
    { 2,  "Hand Axe",     WEAPON_LIGHT,  "Axe",          1, 4,  "Slashing", 1, 4,  "Crushing", false, false, false, true  },
        { 3,  "Longsword",    WEAPON_MEDIUM, "Long Blade",   1, 6,  "Slashing", 1, 6,  "Piercing", false, true,  false, false, { "power_strike", "cleave", "lunge" } },
    { 4,  "Scimitar",     WEAPON_MEDIUM, "Long Blade",   1, 6,  "Slashing", 1, 4,  "Slashing", false, false, false, false },
    { 5,  "Mace",         WEAPON_MEDIUM, "Blunt Weapon", 1, 6,  "Crushing", 1, 4,  "Piercing", false, false, false, false },
    { 6,  "War Axe",      WEAPON_MEDIUM, "Axe",          1, 6,  "Slashing", 1, 4,  "Crushing", false, false, false, false, { "cleave", "power_strike", "crushing_blow" } },
    { 7,  "Spear",        WEAPON_MEDIUM, "Spear",        1, 6,  "Piercing", 1, 4,  "Crushing", false, false, true,  false },
    { 8,  "Estoc",        WEAPON_MEDIUM, "Long Blade",   1, 6,  "Piercing", 1, 4,  "Crushing", false, false, false, false },
    { 9,  "Greatsword",   WEAPON_HEAVY,  "Long Blade",   1, 10, "Slashing", 1, 8,  "Piercing", true,  false, false, false, { "impale", "cleave", "power_strike" } },
    { 10, "Warhammer",    WEAPON_HEAVY,  "Blunt Weapon", 1, 10, "Crushing", 1, 6,  "Piercing", true,  false, false, false },
    { 11, "Battleaxe",    WEAPON_HEAVY,  "Axe",          1, 10, "Slashing", 1, 8,  "Crushing", true,  false, false, false },
    { 12, "Halberd",      WEAPON_HEAVY,  "Spear",        1, 10, "Piercing", 1, 8,  "Slashing", true,  false, true,  false },
    { 13, "Maul",         WEAPON_HEAVY,  "Blunt Weapon", 1, 12, "Crushing", 1, 6,  "Piercing", true,  false, false, false },
    { 14, "Short Bow",    WEAPON_RANGED, "Marksman",     1, 6,  "Piercing", 1, 4,  "Piercing", true,  false, false, false, { "quick_shot", "aimed_shot", "rapid_volley" } },
    { 15, "Long Bow",     WEAPON_RANGED, "Marksman",     1, 6,  "Piercing", 1, 6,  "Piercing", true,  false, false, false, { "quick_shot", "aimed_shot", "piercing_shot" } },
    { 16, "Crossbow",     WEAPON_RANGED, "Marksman",     1, 6,  "Piercing", 1, 6,  "Piercing", true,  false, false, false, { "quick_shot", "aimed_shot" } } 
};

void ResolveWeaponSkillIds()
{
    for (size_t i = 0; i < G_WEAPON_TYPES.size(); i++)
    {
        G_WEAPON_TYPES[i].skillId = FindSkillIdByName(G_WEAPON_TYPES[i].skill);
    }
}