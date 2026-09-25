#include "AbilityHotbar.h"
#include "AbilityData.h"
#include "WeaponData.h"
#include "Durability.h"

static bool ListContains(const std::vector<std::string>& list, const std::string& id)
{
    for (size_t i = 0; i < list.size(); i++)
    {
        if (list[i] == id)
        {
            return true;
        }
    }
    return false;
}

static int ListIndexOf(const std::vector<std::string>& list, const std::string& id)
{
    for (size_t i = 0; i < list.size(); i++)
    {
        if (list[i] == id)
        {
            return (int)i;
        }
    }
    return -1;
}

static bool HotbarContains(const Player& player, const std::string& id)
{
    return ListContains(player.hotbar, id);
}

std::vector<std::string> GetWeaponAbilityList(const Item& weapon)
{
    std::vector<std::string> empty;
    if (weapon.weaponTypeId < 0 || weapon.weaponTypeId >= (int)G_WEAPON_TYPES.size())
    {
        return empty;
    }
    return G_WEAPON_TYPES[weapon.weaponTypeId].abilityIds;
}

static void AddHandToPool(const Item& weapon, std::vector<std::string>& pool)
{
    if (!IsUsable(weapon.condition))
    {
        return; // Broken weapons grant nothing until repaired
    }

    std::vector<std::string> list = GetWeaponAbilityList(weapon);
    for (size_t i = 0; i < list.size(); i++)
    {
        if (FindAbility(list[i]) == nullptr)
        {
            continue; // Typo in a weapon row, skip it instead of crashing the menu
        }
        if (ListContains(pool, list[i]))
        {
            continue;
        }
        pool.push_back(list[i]);
    }
}

std::vector<std::string> BuildAbilityPool(const Player& player)
{
    std::vector<std::string> pool;
    AddHandToPool(player.equippedSlots[SLOT_MAIN_HAND], pool);
    AddHandToPool(player.equippedSlots[SLOT_OFF_HAND], pool);
    return pool;
}

bool IsAbilityAvailable(const Player& player, const std::string& abilityId)
{
    if (abilityId.empty())
    {
        return false;
    }

    const AbilityDef* ability = FindAbility(abilityId);
    if (ability == nullptr)
    {
        return false;
    }

    if (ability->source == ABILITY_SOURCE_WEAPON)
    {
        std::vector<std::string> pool = BuildAbilityPool(player);
        return ListContains(pool, abilityId);
    }

    return false; // Spell and feat abilities have no known-list on Player yet
}

void SyncHotbarAfterEquipChange(Player& player, const Item& oldMainHand, const Item& oldOffHand)
{
    std::vector<std::string> oldMain = GetWeaponAbilityList(oldMainHand);
    std::vector<std::string> oldOff = GetWeaponAbilityList(oldOffHand);
    std::vector<std::string> newMain = GetWeaponAbilityList(player.equippedSlots[SLOT_MAIN_HAND]);
    std::vector<std::string> newOff = GetWeaponAbilityList(player.equippedSlots[SLOT_OFF_HAND]);
    std::vector<std::string> pool = BuildAbilityPool(player);

    for (size_t slot = 0; slot < player.hotbar.size(); slot++)
    {
        std::string current = player.hotbar[slot];
        if (current.empty())
        {
            continue;
        }
        if (ListContains(pool, current))
        {
            continue; // Still usable from the new loadout, leave it alone
        }

        // Orphaned. Find where it sat in the old loadout and take the same position from the new one.
        std::string replacement = "";
        int mainIndex = ListIndexOf(oldMain, current);
        int offIndex = ListIndexOf(oldOff, current);
        if (mainIndex >= 0 && mainIndex < (int)newMain.size())
        {
            replacement = newMain[mainIndex];
        }
        else if (offIndex >= 0 && offIndex < (int)newOff.size())
        {
            replacement = newOff[offIndex];
        }

        // Every failure below leaves the slot as it was. It stays greyed out and wakes up if the old weapon returns.
        if (replacement.empty())
        {
            continue;
        }
        if (!ListContains(pool, replacement))
        {
            continue;
        }
        if (HotbarContains(player, replacement))
        {
            continue;
        }
        player.hotbar[slot] = replacement;
    }
}

void FillEmptyHotbarSlots(Player& player)
{
    std::vector<std::string> pool = BuildAbilityPool(player);
    for (size_t i = 0; i < pool.size(); i++)
    {
        if (HotbarContains(player, pool[i]))
        {
            continue;
        }
        for (size_t slot = 0; slot < player.hotbar.size(); slot++)
        {
            if (player.hotbar[slot].empty())
            {
                player.hotbar[slot] = pool[i];
                break;
            }
        }
    }
}