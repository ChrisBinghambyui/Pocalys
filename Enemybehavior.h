#pragma once
#include <string>
#include <vector>
#include "Enemy.h"
#include "Player.h"
#include "BehaviorData.h"

// Call once per gameplay frame while the world is running. For every living enemy this:
//   1. refreshes its hostile target (detection radius, line of sight, faction stance),
//   2. checks the brain's reflex behaviors, which override everything when they fire,
//   3. otherwise keeps the current behavior, or rolls a new one from the brain's weighted list when
//      the last one finished, timed out, or was interrupted (took damage, gained or lost a target),
//   4. runs the behavior, then moves the enemy through the shared movement step.
// actionMessage is written to when an enemy hits the player.
void UpdateEnemyBehaviors(std::vector<Enemy>& enemies, Player& player, float dt, const PositionFreeFn& isPositionFree, const TileOpaqueFn& isOpaque, const NavMesh& navMesh, const std::vector<Room>& rooms, int& floorAlert, std::string& actionMessage);