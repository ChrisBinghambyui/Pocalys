#pragma once
#include "EnemyData.h"
#include "Item.h"
#include <vector>

int CalculateSpawnWeight(const SpawnRule& rule, int current_floor);
int CalculateScaledHp(const EnemyArchetype& archetype, int current_floor);
int CalculateScaledDamage(const EnemyArchetype& archetype, int current_floor);

// Rolls this archetype's weapon count, type, and material tier into real Item instances.
// Empty vector if the archetype has no weaponTypePool (natural fighters).
std::vector<Item> GenerateEnemyLoadout(const EnemyArchetype& archetype);