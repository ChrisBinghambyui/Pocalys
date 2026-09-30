#pragma once
#include <vector>
#include "Item.h"
#include "Player.h"

const int STASH_PACK_LIMIT = 8; // Stacks a new character can pull out of the stash at the start of a run

// Loot that survives its owner. Extraction banks a character's whole kit here, and a new character
// can pack some of it at the tavern. In-memory only until the save system exists.
struct StashState
{
    std::vector<Item> items;
};

extern StashState G_STASH;

// Adds one item. Ammo of the same type and condition merges into one stack, everything else stays separate
// (a stack of two swords would equip as one sword with quantity 2).
void AddItemToStash(const Item& item);

// Moves the inventory, every equipped item, amulets, and rings into the stash. Returns how many items moved.
int DepositAllToStash(Player& player);

// Moves every stash entry whose flag is true into the player's inventory. flags is parallel to G_STASH.items.
void WithdrawFlaggedFromStash(const std::vector<bool>& flags, Player& player);