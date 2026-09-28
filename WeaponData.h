#pragma once
#include <string>
#include <vector>

enum WeaponCategory {
    WEAPON_LIGHT,
    WEAPON_MEDIUM,
    WEAPON_HEAVY,
    WEAPON_RANGED
};

enum DamageType
{
    DAMAGE_SLASHING,
    DAMAGE_PIERCING,
    DAMAGE_CRUSHING
};

// How well a weapon suits a damage type. Every weapon can attack with all three.
enum AttackRank
{
    ATTACK_RANK_PRIMARY,   // The weapon's primaryDamageType, rolls the primary dice
    ATTACK_RANK_SECONDARY, // The weapon's secondaryDamageType, rolls the secondary dice
    ATTACK_RANK_IMPROVISED // Neither: pommel, haft, flat of the blade. Primary dice at a reduced percent.
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
    std::vector<std::string> abilityIds; // Ordered ids into G_ABILITIES. Position matters, hotbar auto-swap matches on it. Empty = no abilities.
    int skillId = -1; // Index into G_SKILL_TYPES, filled in by ResolveWeaponSkillIds(). -1 if the skill name did not match.
};

extern std::vector<WeaponType> G_WEAPON_TYPES;

// The damage type strings on WeaponType stay the source of truth. These read them on demand.
DamageType ParseDamageType(const std::string& name);
std::string GetDamageTypeName(DamageType type);
AttackRank GetAttackRank(const WeaponType& weapon, DamageType type);

// Call once at startup. Fills skillId on every weapon type by matching its skill name.
void ResolveWeaponSkillIds();