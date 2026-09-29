#include "Combat.h"
#include "EnemyFactory.h"
#include "raylib.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include "Durability.h"
#include "ItemData.h"
#include "SkillScaling.h"

const WeaponType* GetMeleeWeaponType(const Player& attacker)
{
    const Item& weapon = attacker.equippedSlots[SLOT_MAIN_HAND];
    if (weapon.weaponTypeId < 0 || weapon.weaponTypeId >= (int)G_WEAPON_TYPES.size())
    {
        return nullptr;
    }
    const WeaponType& type = G_WEAPON_TYPES[weapon.weaponTypeId];
    if (type.category == WEAPON_RANGED || !IsUsable(weapon.condition))
    {
        return nullptr;
    }
    return &type;
}

int CalculateUnarmedDamage(const Player& attacker)
{
    int damage = attacker.str / 10;
    damage += GetRandomValue(-1, 1);
    if (damage < 1)
    {
        damage = 1;
    }
    return damage;
}

//bool ResolveBumpAttack(Player& attacker, Enemy& defender, std::string& actionMessage)
//{
//    int damage = CalculateUnarmedDamage(attacker);
//    defender.hp -= damage;
//
//    if (defender.hp <= 0)
//    {
//        defender.hp = 0;
//        defender.isDead = true;
//        actionMessage = "The " + GetEnemyDisplayName(defender) + " collapses!";
//        return true;
//    }
//    else
//    {
//        actionMessage = "You hit the " + GetEnemyDisplayName(defender) + " for " + std::to_string(damage) + " damage!";
//        return false;
//    }
//}

// Light materials boost light and ranged weapons, heavy materials boost medium and heavy weapons.
static bool MaterialBonusApplies(MaterialAffinity affinity, WeaponCategory category)
{
    if (affinity == AFFINITY_NONE)
    {
        return true;
    }
    if (affinity == AFFINITY_LIGHT)
    {
        return category == WEAPON_LIGHT || category == WEAPON_RANGED;
    }
    if (affinity == AFFINITY_HEAVY)
    {
        return category == WEAPON_MEDIUM || category == WEAPON_HEAVY;
    }
    return false;
}

static std::string GetAttackVerb(const std::string& damageType)
{
    if (damageType == "Slashing")
    {
        return "slash";
    }
    if (damageType == "Piercing")
    {
        return "stab";
    }
    if (damageType == "Crushing")
    {
        return "smack";
    }
    return "hit";
}

// Phase-1 placeholder: these used to be roll penalties before landing a hit became purely
// positional. Until real-time swing timing exists, they subtract from raw damage instead of
// speed. Convert to swing-speed penalties once the twin-stick conversion adds real attack timing.
const int SHIELD_ATTACK_DAMAGE_PENALTY = 1;
const int DUAL_WIELD_OFFHAND_DAMAGE_PENALTY = 3; // For the future Dual Strike ability. Bump only ever uses the main hand, so this never applies here.

bool IsShieldEquipped(const Player& player)
{
    const Item& offHand = player.equippedSlots[SLOT_OFF_HAND];
    if (offHand.archetypeId.empty())
    {
        return false;
    }
    for (size_t i = 0; i < G_ITEM_ARCHETYPES.size(); i++)
    {
        if (G_ITEM_ARCHETYPES[i].id == offHand.archetypeId)
        {
            return G_ITEM_ARCHETYPES[i].isShield;
        }
    }
    return false;
}

bool IsDualWielding(const Player& player)
{
    const Item& mainHand = player.equippedSlots[SLOT_MAIN_HAND];
    const Item& offHand = player.equippedSlots[SLOT_OFF_HAND];
    if (mainHand.weaponTypeId < 0 || offHand.weaponTypeId < 0)
    {
        return false;
    }
    return true;
}

const float SWING_COOLDOWN_UNARMED = 0.40f;
const float SWING_COOLDOWN_LIGHT = 0.35f;
const float SWING_COOLDOWN_MEDIUM = 0.55f;
const float SWING_COOLDOWN_HEAVY = 0.90f;

float GetAttackCooldownSeconds(const Player& attacker)
{
    const Item& weapon = attacker.equippedSlots[SLOT_MAIN_HAND];
    float baseCooldown = SWING_COOLDOWN_UNARMED;
    int skillId = SKILL_HAND_TO_HAND;

    bool armed = false;
    if (weapon.weaponTypeId >= 0 && weapon.weaponTypeId < (int)G_WEAPON_TYPES.size())
    {
        if (G_WEAPON_TYPES[weapon.weaponTypeId].category != WEAPON_RANGED && IsUsable(weapon.condition))
        {
            armed = true;
        }
    }

    if (armed)
    {
        const WeaponType& type = G_WEAPON_TYPES[weapon.weaponTypeId];
        skillId = type.skillId;
        if (type.category == WEAPON_LIGHT)
        {
            baseCooldown = SWING_COOLDOWN_LIGHT;
        }
        else if (type.category == WEAPON_MEDIUM)
        {
            baseCooldown = SWING_COOLDOWN_MEDIUM;
        }
        else
        {
            baseCooldown = SWING_COOLDOWN_HEAVY;
        }
    }

    int skillLevel = 0;
    if (skillId >= 0 && skillId < (int)attacker.skills.size())
    {
        skillLevel = attacker.skills[skillId].level;
    }

    return baseCooldown / GetSkillSpeedMultiplier(skillLevel);
}

const int IMPROVISED_ATTACK_PERCENT = 35; // Pommel, haft, or flat-of-the-blade attacks land at this percent of a normal hit
const int THRUST_ARMOR_PIERCE = 1;        // Piercing finds the gaps in armor
const int BASH_ARMOR_PIERCE = 2;          // Crushing rattles through plate and shell. Both move onto the ability rows later.

static int GetAttackArmorPierce(DamageType attackType)
{
    if (attackType == DAMAGE_PIERCING)
    {
        return THRUST_ARMOR_PIERCE;
    }
    if (attackType == DAMAGE_CRUSHING)
    {
        return BASH_ARMOR_PIERCE;
    }
    return 0;
}

int RollWeaponDamage(const Item& weapon, DamageType attackType, int str, int agi, bool maxRoll, int ammoBonus)
{
    if (weapon.weaponTypeId < 0 || weapon.weaponTypeId >= (int)G_WEAPON_TYPES.size())
    {
        return 0;
    }
    const WeaponType& type = G_WEAPON_TYPES[weapon.weaponTypeId];

    // Primary type rolls the primary dice, secondary type rolls the secondary dice. Anything else is
    // improvised: primary dice again, scaled down after the bonuses below.
    AttackRank rank = GetAttackRank(type, attackType);
    int diceCount = type.primaryDiceCount;
    int diceSides = type.primaryDiceSides;
    if (rank == ATTACK_RANK_SECONDARY)
    {
        diceCount = type.secondaryDiceCount;
        diceSides = type.secondaryDiceSides;
    }

    int damage = 0;
    if (maxRoll)
    {
        damage = diceCount * diceSides;
    }
    else
    {
        for (int i = 0; i < diceCount; i++)
        {
            damage += GetRandomValue(1, diceSides);
        }
    }

    if (weapon.materialTier >= 0 && weapon.materialTier < (int)G_MATERIAL_TIERS.size())
    {
        const MaterialTier& material = G_MATERIAL_TIERS[weapon.materialTier];
        if (MaterialBonusApplies(material.affinity, type.category))
        {
            damage += material.damageBonus;
        }
    }

    damage += ammoBonus;

    if (type.category == WEAPON_LIGHT || type.category == WEAPON_RANGED)
    {
        damage += agi / 20;
    }
    else
    {
        damage += str / 20;
        if (type.category == WEAPON_HEAVY)
        {
            damage += 1;
        }
    }


    if (rank == ATTACK_RANK_IMPROVISED)
    {
        damage = (damage * IMPROVISED_ATTACK_PERCENT) / 100;
        if (damage < 1)
        {
            damage = 1; // A pommel strike still lands, armor decides whether it matters
        }
    }

    if (damage < 0)
    {
        damage = 0;
    }
    return damage;
}

DamageType ChooseSwingDamageType(const Player& attacker, float moveX, float moveY)
{
    const Item& weapon = attacker.equippedSlots[SLOT_MAIN_HAND];
    DamageType primaryType = DAMAGE_CRUSHING; // Fists
    if (weapon.weaponTypeId >= 0 && weapon.weaponTypeId < (int)G_WEAPON_TYPES.size())
    {
        primaryType = ParseDamageType(G_WEAPON_TYPES[weapon.weaponTypeId].primaryDamageType);
    }

    if (moveX == 0.0f && moveY == 0.0f)
    {
        return primaryType; // Standing still throws the weapon's best swing
    }

    float alignment = moveX * attacker.facingX + moveY * attacker.facingY;
    if (alignment > 0.5f)
    {
        return DAMAGE_PIERCING; // Stepping toward the cursor lunges
    }
    if (alignment < -0.5f)
    {
        return DAMAGE_CRUSHING; // Backing off snaps the pommel or haft out
    }
    return DAMAGE_SLASHING; // Moving across the target swings wide
}

bool ResolveBumpAttack(Player& attacker, Enemy& defender, std::string& actionMessage, DamageType attackType)
{
    std::string enemyName = GetEnemyDisplayName(defender);
    Item& weapon = attacker.equippedSlots[SLOT_MAIN_HAND];

    // Only a working, non-ranged procedural weapon counts. Anything else means fists.
    bool armed = false;
    if (weapon.weaponTypeId >= 0 && weapon.weaponTypeId < (int)G_WEAPON_TYPES.size())
    {
        if (G_WEAPON_TYPES[weapon.weaponTypeId].category != WEAPON_RANGED && IsUsable(weapon.condition))
        {
            armed = true;
        }
    }

    int skillId = SKILL_HAND_TO_HAND;
    std::string verb = "punch";
    if (armed)
    {
        skillId = G_WEAPON_TYPES[weapon.weaponTypeId].skillId;
        verb = GetAttackVerb(GetDamageTypeName(attackType));
    }

    if (skillId < 0 || skillId >= (int)attacker.skills.size())
    {
        actionMessage = "You don't know how to fight with that weapon.";
        return false;
    }

    // Bump attacks always land now: hitting is positional, not a roll. Skill affects what a
    // landed hit does (damage, crit chance) instead of whether it happens. See SkillScaling.h.
    int skillLevel = attacker.skills[skillId].level;
    bool critSuccess = GetRandomValue(1, 100) <= GetSkillCritChancePercent(skillLevel);

    int damage = 0;
    if (armed)
    {
        damage = RollWeaponDamage(weapon, attackType, attacker.str, attacker.agi, critSuccess);
    }
    else
    {
        damage = CalculateUnarmedDamage(attacker);
    }

    damage = (int)(damage * GetSkillDamageMultiplier(skillLevel));

    if (armed && !G_WEAPON_TYPES[weapon.weaponTypeId].twoHanded && IsShieldEquipped(attacker))
    {
        damage -= SHIELD_ATTACK_DAMAGE_PENALTY;
    }

    int armor = GetEnemyArmor(defender);
    if (armed)
    {
        armor -= GetAttackArmorPierce(attackType);
        if (armor < 0)
        {
            armor = 0;
        }
    }
    if (critSuccess)
    {
        armor = 0; // Critical hits ignore AR
    }
    damage -= armor;
    if (damage < 0)
    {
        damage = 0;
    }

    defender.hp -= damage;
    bool killed = false;
    if (defender.hp <= 0)
    {
        defender.hp = 0;
        defender.isDead = true;
        killed = true;
    }

    std::string prefix = "";
    if (critSuccess)
    {
        prefix = "Critical! ";
    }

    if (damage <= 0)
    {
        actionMessage = prefix + "Your " + verb + " glances off the " + enemyName + "'s armor.";
    }
    else if (killed)
    {
        actionMessage = prefix + "You " + verb + " the " + enemyName + " for " + std::to_string(damage) + " damage. It collapses!";
    }
    else
    {
        actionMessage = prefix + "You " + verb + " the " + enemyName + " for " + std::to_string(damage) + " damage!";
    }

    if (armed)
    {
        RollDegradation(weapon);
        if (!IsUsable(weapon.condition))
        {
            actionMessage += " Your weapon breaks!";
        }
    }

    return killed;
}