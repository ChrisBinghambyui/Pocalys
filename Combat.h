#pragma once
#include "Player.h"
#include "Enemy.h"
#include "WeaponData.h"
#include <string>

int CalculateUnarmedDamage(const Player& attacker);

// Raw weapon damage before armor: the dice for attackType (see GetAttackRank), material bonus (only if its affinity fits the weapon),
// STR/AGI bonus, then the condition penalty. maxRoll = true takes maximum dice (critical hits).
// Returns 0 if the item is not a procedural weapon. Enemy attacks can reuse this.
int RollWeaponDamage(const Item& weapon, DamageType attackType, int str, int agi, bool maxRoll, int ammoBonus = 0);

// True if the off-hand slot holds an item flagged as a shield in G_ITEM_ARCHETYPES.
bool IsShieldEquipped(const Player& player);

// True if both hands hold a procedural weapon. Used by the future Dual Strike ability's off-hand penalty, not by bump.
bool IsDualWielding(const Player& player);

// Seconds between swings for whatever is in the main hand, shortened by weapon skill. Ranged weapons and
// broken weapons use fist timing, matching what ResolveBumpAttack actually swings with.
float GetAttackCooldownSeconds(const Player& attacker);

// Which damage type a left-click swing throws. Pass the normalized WASD input vector.
// Standing still: the main-hand weapon's primary type (its best swing).
// Moving toward the cursor: piercing (thrust). Moving across it: slashing. Moving away: crushing (pommel or haft).
DamageType ChooseSwingDamageType(const Player& attacker, float moveX, float moveY);

bool ResolveBumpAttack(Player& attacker, Enemy& defender, std::string& actionMessage, DamageType attackType);