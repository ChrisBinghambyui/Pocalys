#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Enemy.h"
#include "Player.h"
#include "LineOfSight.h"
#include "NavMesh.h"
#include "DungeonData.h" // For Room, used by Patrol

// Shared enemy tuning, all in tile units.
const float ENEMY_BASE_MOVE_SPEED = 3.0f;   // Tiles per second at archetype speed 100. The player moves at 5.
const float ENEMY_COLLISION_RADIUS = 0.3f;  // Same wall check the player uses
const float ENEMY_MELEE_RANGE = 1.0f;       // Distance between positions at which a swing lands
const float ENEMY_BODY_SPACING = 0.6f;      // Bodies refuse to step closer than this to each other

// center x, center y, radius. True if a body of that size fits there.
using PositionFreeFn = std::function<bool(float, float, float)>;

// Everything a behavior can see or touch for one enemy on one frame.
struct AIContext
{
    Enemy& self;
    std::vector<Enemy>& enemies;
    Player& player;
    float dt;
    const PositionFreeFn& isPositionFree;
    const TileOpaqueFn& isOpaque;
    const NavMesh& navMesh;
    int& floorAlert;
    const std::vector<Room>& rooms;
    std::string& actionMessage;
};

enum BehaviorStatus
{
    BEHAVIOR_RUNNING,
    BEHAVIOR_FINISHED
};

// One small piece of action, defined once. Brains (BrainData.h) pick from these by id.
// Behaviors never move an enemy directly: they set Enemy.moveDirX/moveDirY/moveSpeedScale and the
// shared movement step in EnemyBehavior.cpp does the walls and collision.
struct BehaviorDef
{
    std::string id;
    std::string name;
    float maxDuration; // Seconds before the behavior gives up. 0 = no limit. start() may override it on the enemy.
    float (*score)(const AIContext& ctx);      // Context multiplier. 0 = can't apply right now, 1 = normal, above 1 = urgent.
    void (*start)(const AIContext& ctx);       // Runs once when picked.
    BehaviorStatus(*update)(const AIContext& ctx); // Runs every frame while active.
};

extern std::vector<BehaviorDef> G_BEHAVIORS;

// Null if no behavior has this id.
const BehaviorDef* FindBehavior(const std::string& id);

// True if the enemy currently has a hostile target (the player or another enemy).
bool EnemyHasTarget(const Enemy& enemy);

// Position of the current target, same units as Enemy.x/y. False if there is no valid target.
// Raw form, usable outside an AIContext (Squad.cpp needs it before behaviors run this frame).
bool GetTargetPosition(const Enemy& enemy, const std::vector<Enemy>& enemies, const Player& player, float& outX, float& outY);
bool GetTargetPosition(const AIContext& ctx, float& outX, float& outY);

void StopEnemyMovement(Enemy& enemy);

// Requests a new path from the enemy's current position to (goalX, goalY) over the shared navmesh
// and stores it on the enemy, replacing whatever it was following. Leaves the path empty if no
// route exists, which callers should treat as "fall back to walking straight at the target".
void RequestPath(const AIContext& ctx, float goalX, float goalY);

// Steers the enemy toward its next stored waypoint, popping waypoints as they're reached.
// Returns true once every waypoint has been consumed.
bool FollowStoredPath(Enemy& enemy, float speedScale);