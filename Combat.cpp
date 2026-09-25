#include "Combat.h"
#include "EnemyFactory.h"
#include "raylib.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include "Durability.h"
#include "ItemData.h"

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

const int SHIELD_ATTACK_ROLL_PENALTY = 5;
const int DUAL_WIELD_OFFHAND_ROLL_PENALTY = 20; // For the future Dual Strike ability. Bump only ever uses the main hand, so this never applies here.

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

int RollWeaponDamage(const Item& weapon, int str, int agi, bool maxRoll, int ammoBonus)
{
    if (weapon.weaponTypeId < 0 || weapon.weaponTypeId >= (int)G_WEAPON_TYPES.size())
    {
        return 0;
    }
    const WeaponType& type = G_WEAPON_TYPES[weapon.weaponTypeId];

    int damage = 0;
    if (maxRoll)
    {
        damage = type.primaryDiceCount * type.primaryDiceSides;
    }
    else
    {
        for (int i = 0; i < type.primaryDiceCount; i++)
        {
            damage += GetRandomValue(1, type.primaryDiceSides);
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

    damage += GetConditionDamagePenalty(weapon.condition);

    if (damage < 0)
    {
        damage = 0;
    }
    return damage;
}

bool ResolveBumpAttack(Player& attacker, Enemy& defender, std::string& actionMessage)
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
        verb = GetAttackVerb(G_WEAPON_TYPES[weapon.weaponTypeId].primaryDamageType);
    }

    if (skillId < 0 || skillId >= (int)attacker.skills.size())
    {
        actionMessage = "You don't know how to fight with that weapon.";
        return false;
    }

    int rollPenalty = 0;
    if (armed)
    {
        rollPenalty += GetConditionRollPenalty(weapon.condition);
        if (!G_WEAPON_TYPES[weapon.weaponTypeId].twoHanded && IsShieldEquipped(attacker))
        {
            rollPenalty += SHIELD_ATTACK_ROLL_PENALTY;
        }
    }

    // Roll under: effective skill is skill plus Luck bonus. 1-2 always hits, 98-99 always misses.
    int effectiveSkill = attacker.skills[skillId].level + (attacker.lck / 10);
    int roll = GetRandomValue(1, 100);
    bool critSuccess = false;
    bool critFail = false;
    bool hit = false;
    if (roll <= 2)
    {
        critSuccess = true;
        hit = true;
    }
    else if (roll >= 98)
    {
        critFail = true;
    }
    else if (roll + rollPenalty <= effectiveSkill)
    {
        hit = true;
    }

    if (critFail)
    {
        actionMessage = "You fumble your attack!";
        return false;
    }
    if (!hit)
    {
        actionMessage = "You " + verb + " at the " + enemyName + " and miss.";
        return false;
    }

    int damage = 0;
    if (armed)
    {
        damage = RollWeaponDamage(weapon, attacker.str, attacker.agi, critSuccess);
    }
    else
    {
        damage = CalculateUnarmedDamage(attacker);
    }

    int armor = GetEnemyArmor(defender);
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