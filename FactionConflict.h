#pragma once
#include "Enemy.h"
#include <vector>

// Called when the player returns to a previously-generated floor. Groups living enemies by their
// primary (highest-priority) faction, resolves hostile-pair casualties by aggregate power (summed HP),
// and may spawn scavengers near fresh corpses. Mutates enemies in place: kills are isDead = true, hp = 0.
// floorNumber is 1-based, matching CreateEnemy's spawnFloor and GenerateFloor's spawn-rule lookups.
void ResolveOffscreenFactionConflicts(std::vector<Enemy>& enemies, int floorNumber);
