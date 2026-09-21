#pragma once
#include <string>
#include <vector>

enum WeaponCategory {
    WEAPON_LIGHT,
    WEAPON_MEDIUM,
    WEAPON_HEAVY,
    WEAPON_RANGED
};

struct WeaponType
{
    int id; // Matches the loose numbering you were sketching (2=dagger, 3=sword, etc.)
    std::string name;
    WeaponCategory category;
    std::string skill;          // Governing skill, e.g. "Short Blade", "Long Blade"
    int primaryDiceCount;
    int primaryDiceSides;
    std::string primaryDamageType;   // "Slashing", "Piercing", "Crushing"
    int secondaryDiceCount;
    int secondaryDiceSides;
    std::string secondaryDamageType;
    bool twoHanded;
    bool versatile;  // Can be used one- or two-handed
    bool reach;
    bool thrown;
};

extern std::vector<WeaponType> G_WEAPON_TYPES;