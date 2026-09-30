#include "StashData.h"

StashState G_STASH;

static bool CanStack(const Item& a, const Item& b)
{
    if (a.ammoTypeId < 0 || b.ammoTypeId < 0)
    {
        return false;
    }
    if (a.ammoTypeId != b.ammoTypeId)
    {
        return false;
    }
    if (a.condition != b.condition)
    {
        return false;
    }
    return true;
}

void AddItemToStash(const Item& item)
{
    if (item.IsEmpty())
    {
        return;
    }

    for (size_t i = 0; i < G_STASH.items.size(); i++)
    {
        if (CanStack(G_STASH.items[i], item))
        {
            G_STASH.items[i].quantity += item.quantity;
            return;
        }
    }
    G_STASH.items.push_back(item);
}

int DepositAllToStash(Player& player)
{
    int movedCount = 0;

    for (size_t i = 0; i < player.inventory.size(); i++)
    {
        AddItemToStash(player.inventory[i]);
        movedCount++;
    }
    player.inventory.clear();

    for (int s = 0; s < SLOT_SINGLE_COUNT; s++)
    {
        if (!player.equippedSlots[s].IsEmpty())
        {
            AddItemToStash(player.equippedSlots[s]);
            player.equippedSlots[s] = Item();
            movedCount++;
        }
    }

    for (size_t i = 0; i < player.equippedAmulets.size(); i++)
    {
        AddItemToStash(player.equippedAmulets[i]);
        movedCount++;
    }
    player.equippedAmulets.clear();

    for (size_t i = 0; i < player.equippedRings.size(); i++)
    {
        AddItemToStash(player.equippedRings[i]);
        movedCount++;
    }
    player.equippedRings.clear();

    return movedCount;
}

void WithdrawFlaggedFromStash(const std::vector<bool>& flags, Player& player)
{
    // Back to front so erasing never shifts an entry we have yet to visit
    for (int i = (int)G_STASH.items.size() - 1; i >= 0; i--)
    {
        if (i < (int)flags.size() && flags[i])
        {
            player.inventory.push_back(G_STASH.items[i]);
            G_STASH.items.erase(G_STASH.items.begin() + i);
        }
    }
}