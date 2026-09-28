#pragma once
#include "Item.h"

// Current damage/AR penalty for this item's condition tier, per the rulebook's table.
int GetConditionDamagePenalty(ConditionTier condition);
int GetConditionArmorPenalty(ConditionTier condition);
bool IsUsable(ConditionTier condition); // False once Broken

// Rolls this item's material-specific degradation chance. Call once per landed hit
// (weapons) or per hit taken (armor). Drops one condition tier on a hit, floors at Broken.
void RollDegradation(Item& item);