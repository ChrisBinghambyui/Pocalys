#pragma once
#include "Player.h"
#include "Enemy.h"
#include <string>

int CalculateUnarmedDamage(const Player& attacker);

// Raw weapon damage before armor: primary dice, material bonus (only if its affinity fits the weapon),
// STR/AGI bonus, then the condition penalty. maxRoll = true takes maximum dice (critical hits).
// Returns 0 if the item is not a procedural weapon. Enemy attacks can reuse this.
int RollWeaponDamage(const Item& weapon, int str, int agi, bool maxRoll, int ammoBonus = 0);

// True if the off-hand slot holds an item flagged as a shield in G_ITEM_ARCHETYPES.
bool IsShieldEquipped(const Player& player);

// True if both hands hold a procedural weapon. Used by the future Dual Strike ability's off-hand penalty, not by bump.
bool IsDualWielding(const Player& player);

bool ResolveBumpAttack(Player& attacker, Enemy& defender, std::string& actionMessage);