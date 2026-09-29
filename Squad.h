#pragma once
#include <vector>
#include "Enemy.h"
#include "Player.h"

// Groups living, currently-hostile enemies into squads (same primary faction, same target, within
// clustering range of each other, chained through intermediate members) and assigns each member a
// formation slot based on its archetype's EnemyRole. Frontline forms a shield-wall arc at melee
// range facing the target, skirmishers approach from a wide flank angle at the same range, and
// backline holds well behind. Solo enemies (no one else nearby to squad up with) are left with
// hasSquadSlot false so their brain's plain Hunt behavior takes over instead.
//
// Call once per frame, before UpdateEnemyBehaviors so behaviors see this frame's slots.
void UpdateSquads(std::vector<Enemy>& enemies, Player& player);