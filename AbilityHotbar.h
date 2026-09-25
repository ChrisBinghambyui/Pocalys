#pragma once
#include <string>
#include <vector>
#include "Player.h"
#include "Item.h"

// Ordered ability ids a weapon item grants. Empty for empty hands, archetype items, and weapon types with no abilities.
std::vector<std::string> GetWeaponAbilityList(const Item& weapon);

// Everything usable right now: main hand then off hand, broken weapons skipped, no duplicates.
// This is the list the abilities menu should show.
std::vector<std::string> BuildAbilityPool(const Player& player);

// The single gate for using an ability. Check this at use time, never trust the hotbar alone.
bool IsAbilityAvailable(const Player& player, const std::string& abilityId);

// Call after ANY change to main hand or off hand. Pass the items that were there BEFORE the change.
void SyncHotbarAfterEquipChange(Player& player, const Item& oldMainHand, const Item& oldOffHand);

// Puts every pool ability into the first empty slots. Meant for the start of a run.
void FillEmptyHotbarSlots(Player& player);