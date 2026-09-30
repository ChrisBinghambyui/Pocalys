#pragma once
#include <vector>
#include "DungeonData.h"
#include "Enemy.h"
#include "ExpeditionData.h"

// Fills `enemies` for a freshly generated floor. Clears it first.
// Each room (except room 0, the arrival room) gets a group whose organization level is rolled from the
// floor's chunk: 0 stragglers, 1 packs, 2 squads (coordinated), 3 warbands (squad plus a lord/master leader).
// If the expedition rolls a hidden minority, one remote room holds a group of a faction hostile to the theme.
// downStairRoomIndex is the room holding the down stairs, used only to pick a room far from both stairs.
void PopulateFloor(const FloorParams& params, const std::vector<Room>& rooms, int downStairRoomIndex, std::vector<Enemy>& enemies);