#pragma once
#include "Enemy.h"
#include <vector>

// Energy an actor needs before it gets to act. Enemy speed is energy gained per player action.
const int ACTION_THRESHOLD = 100;

// Call once after the player completes a turn-consuming action (move or attack).
// Grants energy to every living enemy and returns pointers to the ones ready to act,
// in the order they became ready. A fast enemy (speed > ACTION_THRESHOLD) can appear more than once.
std::vector<Enemy*> AdvanceEnemyEnergy(std::vector<Enemy>& enemies);