#pragma once
#include "Enemy.h"
#include "Player.h"
#include "LineOfSight.h"
#include <vector>
#include <functional>

enum AIBrain
{
    BRAIN_MINDLESS,     // Undead, constructs. Always advances on the nearest hostile target, never flees, no allies needed.
    BRAIN_PREDATOR,     // Beasts. Hunts the nearest hostile target. Flees when badly hurt and alone, fights on with allies nearby.
    BRAIN_SKITTISH,     // Vermin, scavengers. Avoids hostile contact, only closes in when cornered.
    BRAIN_TERRITORIAL,  // Guardians. Holds a home tile, only aggros within its leash range, returns home once clear.
    BRAIN_CASTER        // Casters. Keeps its distance from hostile targets, backs away if they close in.
};

enum AIActionType
{
    AI_ACTION_IDLE,
    AI_ACTION_MOVE,
    AI_ACTION_ATTACK_ENEMY,
    AI_ACTION_ATTACK_PLAYER
};

struct AIAction
{
    AIActionType type = AI_ACTION_IDLE;
    int moveX = 0;
    int moveY = 0;
    Enemy* targetEnemy = nullptr;
};

// Predicate: true if (x, y) is a tile an enemy can step onto (in bounds, not a wall, not occupied).
using TileWalkableFn = std::function<bool(int, int)>;

// Decides what a single ready enemy does this tick. Does not mutate anything, just picks the action.
// homeX/homeY is the enemy's spawn tile, used by BRAIN_TERRITORIAL's leash.
AIAction DecideEnemyAction(Enemy& actor, int homeX, int homeY, std::vector<Enemy>& enemies, Player& player, const TileWalkableFn& isWalkable, const TileOpaqueFn& isOpaque);

// Unarmed-style damage from an enemy's STR against a defender's armor. Shared by both attack kinds
// until enemies get real equipped-weapon combat. Returns damage actually dealt, floored at 0.
int CalculateEnemyAttackDamage(const Enemy& attacker, int defenderArmor);
