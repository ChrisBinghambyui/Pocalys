#include "BehaviorData.h"
#include "Alert.h"
#include "EnemyFactory.h"
#include "raylib.h"
#include <cmath>

static const float WANDER_MIN_DISTANCE = 2.0f;
static const float WANDER_MAX_DISTANCE = 5.0f;
static const float ARRIVE_DISTANCE = 0.2f;
static const float FLEE_HEALTH_RATIO = 0.3f;
static const float FLEE_SAFE_DISTANCE = 10.0f;
static const float BASE_ATTACK_COOLDOWN = 1.0f;     // Seconds between swings at archetype speed 100
static const float ATTACK_LEASH_MULTIPLIER = 1.3f;  // A swing recovery is dropped if the target gets this far past melee range
static const float THREAT_PRESENT_IDLE_SCORE = 0.1f; // Idle and Wander barely register while there is something to fight
static const float PATH_ARRIVE_DISTANCE = 0.25f;
static const float HUNT_REPATH_INTERVAL_MIN = 0.4f;
static const float HUNT_REPATH_INTERVAL_MAX = 0.8f;
static const float HUNT_TARGET_MOVE_REPATH_DISTANCE = 1.5f; // Repath early once the target has drifted this far from where the last path was aimed

// ---------- Shared helpers ----------

bool EnemyHasTarget(const Enemy& enemy)
{
    if (enemy.targetIsPlayer)
    {
        return true;
    }
    if (enemy.targetIndex >= 0)
    {
        return true;
    }
    return false;
}

bool GetTargetPosition(const Enemy& enemy, const std::vector<Enemy>& enemies, const Player& player, float& outX, float& outY)
{
    if (enemy.targetIsPlayer)
    {
        outX = player.x;
        outY = player.y;
        return true;
    }
    if (enemy.targetIndex >= 0 && enemy.targetIndex < (int)enemies.size())
    {
        const Enemy& target = enemies[enemy.targetIndex];
        if (target.isDead)
        {
            return false;
        }
        outX = target.x;
        outY = target.y;
        return true;
    }
    return false;
}

bool GetTargetPosition(const AIContext& ctx, float& outX, float& outY)
{
    return GetTargetPosition(ctx.self, ctx.enemies, ctx.player, outX, outY);
}
void RequestPath(const AIContext& ctx, float goalX, float goalY)
{
    Enemy& self = ctx.self;
    self.path = ctx.navMesh.FindPath(self.x, self.y, goalX, goalY, ENEMY_COLLISION_RADIUS);
    self.pathIndex = 0;
}

bool FollowStoredPath(Enemy& enemy, float speedScale)
{
    if (enemy.pathIndex >= (int)enemy.path.size())
    {
        StopEnemyMovement(enemy);
        return true;
    }

    Vector2 waypoint = enemy.path[enemy.pathIndex];
    float dx = waypoint.x - enemy.x;
    float dy = waypoint.y - enemy.y;
    float distance = sqrtf(dx * dx + dy * dy);

    if (distance <= PATH_ARRIVE_DISTANCE)
    {
        enemy.pathIndex++;
        if (enemy.pathIndex >= (int)enemy.path.size())
        {
            StopEnemyMovement(enemy);
            return true;
        }
        waypoint = enemy.path[enemy.pathIndex];
        dx = waypoint.x - enemy.x;
        dy = waypoint.y - enemy.y;
        distance = sqrtf(dx * dx + dy * dy);
    }

    if (distance < 0.001f)
    {
        StopEnemyMovement(enemy);
        return false;
    }

    enemy.moveDirX = dx / distance;
    enemy.moveDirY = dy / distance;
    enemy.moveSpeedScale = speedScale;
    return false;
}

void StopEnemyMovement(Enemy& enemy)
{
    enemy.moveDirX = 0.0f;
    enemy.moveDirY = 0.0f;
}

static float GetTargetDistance(const AIContext& ctx)
{
    float targetX = 0.0f;
    float targetY = 0.0f;
    if (!GetTargetPosition(ctx, targetX, targetY))
    {
        return -1.0f;
    }
    float dx = targetX - ctx.self.x;
    float dy = targetY - ctx.self.y;
    return sqrtf(dx * dx + dy * dy);
}

static void SetMoveToward(Enemy& enemy, float targetX, float targetY, float speedScale)
{
    float dx = targetX - enemy.x;
    float dy = targetY - enemy.y;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 0.001f)
    {
        StopEnemyMovement(enemy);
        return;
    }
    enemy.moveDirX = dx / length;
    enemy.moveDirY = dy / length;
    enemy.moveSpeedScale = speedScale;
}

static void SetMoveAway(Enemy& enemy, float fromX, float fromY, float speedScale)
{
    float dx = enemy.x - fromX;
    float dy = enemy.y - fromY;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < 0.001f)
    {
        StopEnemyMovement(enemy);
        return;
    }
    enemy.moveDirX = dx / length;
    enemy.moveDirY = dy / length;
    enemy.moveSpeedScale = speedScale;
}

// Unarmed-style damage from STR, same shape the old EnemyAi.cpp used. Swap for weapon damage later.
static int RollEnemyMeleeDamage(const Enemy& attacker, int defenderArmor)
{
    int damage = attacker.str / 10;
    damage += GetRandomValue(-1, 1);
    if (damage < 1)
    {
        damage = 1;
    }
    damage -= defenderArmor;
    if (damage < 0)
    {
        damage = 0;
    }
    return damage;
}

static void LandMeleeHit(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    std::string attackerName = GetEnemyDisplayName(self);

    if (self.targetIsPlayer)
    {
        int damage = RollEnemyMeleeDamage(self, 0); // Player armor stays 0 until a GetPlayerArmor exists
        ctx.player.hp -= damage;
        if (ctx.player.hp < 0)
        {
            ctx.player.hp = 0;
        }
        if (damage > 0)
        {
            ctx.actionMessage = "The " + attackerName + " hits you for " + std::to_string(damage) + " damage!";
        }
        else
        {
            ctx.actionMessage = "The " + attackerName + "'s blow glances off you.";
        }
        return;
    }

    if (self.targetIndex >= 0 && self.targetIndex < (int)ctx.enemies.size())
    {
        Enemy& target = ctx.enemies[self.targetIndex];
        int damage = RollEnemyMeleeDamage(self, GetEnemyArmor(target));
        target.hp -= damage;
        if (target.hp <= 0)
        {
            target.hp = 0;
            target.isDead = true;
        }
    }
}

// ---------- Idle ----------

static float ScoreIdle(const AIContext& ctx)
{
    if (EnemyHasTarget(ctx.self))
    {
        return THREAT_PRESENT_IDLE_SCORE;
    }
    if (IsFloorWary(ctx.floorAlert))
    {
        return 0.0f; // The dungeon doesn't stand around once it's been stirred up
    }
    return 1.0f;
}

static void StartIdle(const AIContext& ctx)
{
    StopEnemyMovement(ctx.self);
    ctx.self.behaviorLimit = (float)GetRandomValue(10, 30) / 10.0f; // Stand around for 1 to 3 seconds
}

static BehaviorStatus UpdateIdle(const AIContext& ctx)
{
    StopEnemyMovement(ctx.self);
    return BEHAVIOR_RUNNING; // The timer ends it
}

// ---------- Wander ----------

static float ScoreWander(const AIContext& ctx)
{
    if (EnemyHasTarget(ctx.self))
    {
        return THREAT_PRESENT_IDLE_SCORE;
    }
    if (IsFloorWary(ctx.floorAlert))
    {
        return 0.0f; // Patrol takes over: aimless wandering isn't what a wary floor does
    }
    return 1.0f;
}

static void StartWander(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    self.hasGoal = false;

    for (int attempt = 0; attempt < 6; attempt++)
    {
        float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
        float spread = (float)GetRandomValue(0, 100) / 100.0f;
        float distance = WANDER_MIN_DISTANCE + spread * (WANDER_MAX_DISTANCE - WANDER_MIN_DISTANCE);
        float goalX = self.x + cosf(angle) * distance;
        float goalY = self.y + sinf(angle) * distance;
        if (ctx.isPositionFree(goalX + 0.5f, goalY + 0.5f, ENEMY_COLLISION_RADIUS))
        {
            self.goalX = goalX;
            self.goalY = goalY;
            self.hasGoal = true;
            return;
        }
    }
}

static BehaviorStatus UpdateWander(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    if (!self.hasGoal)
    {
        return BEHAVIOR_FINISHED;
    }

    float dx = self.goalX - self.x;
    float dy = self.goalY - self.y;
    if (sqrtf(dx * dx + dy * dy) <= ARRIVE_DISTANCE)
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    SetMoveToward(self, self.goalX, self.goalY, 0.5f);
    return BEHAVIOR_RUNNING;
}

// ---------- Hunt ----------

static float ScoreHunt(const AIContext& ctx)
{
    if (ctx.self.hasSquadSlot)
    {
        return 0.0f; // Formation movement takes over when part of a coordinated squad
    }
    float distance = GetTargetDistance(ctx);
    if (distance < 0.0f)
    {
        return 0.0f; // Nothing to hunt
    }
    if (distance <= ENEMY_MELEE_RANGE)
    {
        return 0.0f; // Already close enough, MeleeAttack takes over
    }
    return 1.0f;
}

static void StartHunt(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    self.moveSpeedScale = 1.0f;
    self.path.clear();
    self.pathIndex = 0;
    self.pathTargetX = self.x; // Forces the drift check below to fire on the first update tick
    self.pathTargetY = self.y;
    self.repathTimer = 0.0f;
}

static BehaviorStatus UpdateHunt(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    float targetX = 0.0f;
    float targetY = 0.0f;
    if (!GetTargetPosition(ctx, targetX, targetY))
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    float directDx = targetX - self.x;
    float directDy = targetY - self.y;
    if (sqrtf(directDx * directDx + directDy * directDy) <= ENEMY_MELEE_RANGE)
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    self.repathTimer -= ctx.dt;
    float driftDx = targetX - self.pathTargetX;
    float driftDy = targetY - self.pathTargetY;
    float drift = sqrtf(driftDx * driftDx + driftDy * driftDy);

    if (self.repathTimer <= 0.0f || drift >= HUNT_TARGET_MOVE_REPATH_DISTANCE)
    {
        RequestPath(ctx, targetX, targetY);
        self.pathTargetX = targetX;
        self.pathTargetY = targetY;
        float spread = (float)GetRandomValue(0, 100) / 100.0f;
        self.repathTimer = HUNT_REPATH_INTERVAL_MIN + spread * (HUNT_REPATH_INTERVAL_MAX - HUNT_REPATH_INTERVAL_MIN);
    }

    if (self.path.empty())
    {
        SetMoveToward(self, targetX, targetY, 1.0f); // No route found (disconnected geometry), walk straight rather than freeze
        return BEHAVIOR_RUNNING;
    }

    FollowStoredPath(self, 1.0f);
    return BEHAVIOR_RUNNING;
}

// ---------- Melee Attack ----------

static float ScoreMeleeAttack(const AIContext& ctx)
{
    float distance = GetTargetDistance(ctx);
    if (distance < 0.0f)
    {
        return 0.0f;
    }
    if (distance > ENEMY_MELEE_RANGE)
    {
        return 0.0f;
    }
    return 1.0f;
}

static void StartMeleeAttack(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    StopEnemyMovement(self);
    if (self.attackCooldown > 0.0f)
    {
        return; // Still recovering from the last swing, update() just waits it out
    }

    self.attackCooldown = BASE_ATTACK_COOLDOWN * 100.0f / (float)GetEnemySpeed(self);
    LandMeleeHit(ctx);
}

static BehaviorStatus UpdateMeleeAttack(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    StopEnemyMovement(self);

    float distance = GetTargetDistance(ctx);
    if (distance < 0.0f || distance > ENEMY_MELEE_RANGE * ATTACK_LEASH_MULTIPLIER)
    {
        return BEHAVIOR_FINISHED;
    }
    if (self.attackCooldown > 0.0f)
    {
        return BEHAVIOR_RUNNING;
    }
    return BEHAVIOR_FINISHED;
}

// ---------- Flee (reflex) ----------

static const float FLEE_ALLY_CHECK_RADIUS = 4.0f;

static int CountNearbyAllies(const AIContext& ctx)
{
    int count = 0;
    for (const auto& other : ctx.enemies)
    {
        if (&other == &ctx.self || other.isDead)
        {
            continue;
        }
        if (GetStance(ctx.self, other) != STANCE_ALLIED)
        {
            continue;
        }
        float dx = other.x - ctx.self.x;
        float dy = other.y - ctx.self.y;
        if (sqrtf(dx * dx + dy * dy) <= FLEE_ALLY_CHECK_RADIUS)
        {
            count++;
        }
    }
    return count;
}

static float ScoreFlee(const AIContext& ctx)
{
    if (!EnemyHasTarget(ctx.self))
    {
        return 0.0f;
    }
    if (ctx.self.maxHp <= 0)
    {
        return 0.0f;
    }
    float healthRatio = (float)ctx.self.hp / (float)ctx.self.maxHp;
    if (healthRatio > FLEE_HEALTH_RATIO)
    {
        return 0.0f;
    }
    if (CountNearbyAllies(ctx) > 0)
    {
        return 0.0f; // Allies nearby to lean on, hold the line instead of running
    }
    return 1.0f + (FLEE_HEALTH_RATIO - healthRatio); // Hurt worse, more urgent. Only matters if a brain rolls it instead of using it as a reflex.
}

static void StartFlee(const AIContext& ctx)
{
    ctx.self.moveSpeedScale = 1.1f;
    ctx.self.path.clear();
    ctx.self.pathIndex = 0;
    ctx.self.repathTimer = 0.0f;
}

static BehaviorStatus UpdateFlee(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    float targetX = 0.0f;
    float targetY = 0.0f;
    if (!GetTargetPosition(ctx, targetX, targetY))
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    float dx = self.x - targetX;
    float dy = self.y - targetY;
    float distance = sqrtf(dx * dx + dy * dy);
    if (distance >= FLEE_SAFE_DISTANCE)
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    self.repathTimer -= ctx.dt;
    if (self.repathTimer <= 0.0f || self.path.empty())
    {
        float awayLength = distance;
        if (awayLength < 0.001f)
        {
            awayLength = 1.0f;
        }
        float awayX = self.x + (dx / awayLength) * FLEE_SAFE_DISTANCE;
        float awayY = self.y + (dy / awayLength) * FLEE_SAFE_DISTANCE;
        RequestPath(ctx, awayX, awayY);
        float spread = (float)GetRandomValue(0, 100) / 100.0f;
        self.repathTimer = HUNT_REPATH_INTERVAL_MIN + spread * (HUNT_REPATH_INTERVAL_MAX - HUNT_REPATH_INTERVAL_MIN);
    }

    if (self.path.empty())
    {
        SetMoveAway(self, targetX, targetY, 1.1f);
        return BEHAVIOR_RUNNING;
    }

    FollowStoredPath(self, 1.1f);
    return BEHAVIOR_RUNNING;
}

// ---------- Follow Squad Order ----------

static float ScoreSquadOrder(const AIContext& ctx)
{
    if (!ctx.self.hasSquadSlot)
    {
        return 0.0f;
    }
    float dx = ctx.self.squadSlotX - ctx.self.x;
    float dy = ctx.self.squadSlotY - ctx.self.y;
    if (sqrtf(dx * dx + dy * dy) <= PATH_ARRIVE_DISTANCE)
    {
        return 0.0f; // Already in position
    }
    return 1.0f;
}

static void StartSquadOrder(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    self.moveSpeedScale = 1.0f;
    self.path.clear();
    self.pathIndex = 0;
    self.repathTimer = 0.0f;
}

static BehaviorStatus UpdateSquadOrder(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    if (!self.hasSquadSlot)
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    float dx = self.squadSlotX - self.x;
    float dy = self.squadSlotY - self.y;
    if (sqrtf(dx * dx + dy * dy) <= PATH_ARRIVE_DISTANCE)
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    self.repathTimer -= ctx.dt;
    if (self.repathTimer <= 0.0f || self.path.empty())
    {
        RequestPath(ctx, self.squadSlotX, self.squadSlotY);
        float spread = (float)GetRandomValue(0, 100) / 100.0f;
        self.repathTimer = HUNT_REPATH_INTERVAL_MIN + spread * (HUNT_REPATH_INTERVAL_MAX - HUNT_REPATH_INTERVAL_MIN);
    }

    if (self.path.empty())
    {
        SetMoveToward(self, self.squadSlotX, self.squadSlotY, 1.0f);
        return BEHAVIOR_RUNNING;
    }

    FollowStoredPath(self, 1.0f);
    return BEHAVIOR_RUNNING;
}

// ---------- Patrol ----------

static const float PATROL_JOIN_RADIUS = 8.0f; // Same-faction allies already headed somewhere within this range get joined instead of picking a fresh room

static std::string GetPatrolFactionId(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr || archetype->factionIds.empty())
    {
        return "";
    }
    return archetype->factionIds[0];
}

static int PickPatrolRoomIndex(const AIContext& ctx)
{
    std::string myFaction = GetPatrolFactionId(ctx.self);
    if (!myFaction.empty())
    {
        for (const auto& other : ctx.enemies)
        {
            if (&other == &ctx.self || other.isDead)
            {
                continue;
            }
            if (other.patrolRoomIndex < 0)
            {
                continue;
            }
            if (GetPatrolFactionId(other) != myFaction)
            {
                continue;
            }
            float dx = other.x - ctx.self.x;
            float dy = other.y - ctx.self.y;
            if (sqrtf(dx * dx + dy * dy) <= PATROL_JOIN_RADIUS)
            {
                return other.patrolRoomIndex; // Join the pack instead of scattering
            }
        }
    }

    int currentRoom = -1;
    float bestCurrentDistSq = 1000000.0f;
    for (size_t i = 0; i < ctx.rooms.size(); i++)
    {
        float dx = (float)ctx.rooms[i].centerX() - ctx.self.x;
        float dy = (float)ctx.rooms[i].centerY() - ctx.self.y;
        float distSq = dx * dx + dy * dy;
        if (distSq < bestCurrentDistSq)
        {
            bestCurrentDistSq = distSq;
            currentRoom = (int)i;
        }
    }

    if (ctx.rooms.size() <= 1)
    {
        return 0;
    }

    int roomIndex = currentRoom;
    while (roomIndex == currentRoom)
    {
        roomIndex = GetRandomValue(0, (int)ctx.rooms.size() - 1);
    }
    return roomIndex;
}

static float ScorePatrol(const AIContext& ctx)
{
    if (EnemyHasTarget(ctx.self))
    {
        return 0.0f; // Hunt, formation, or attack takes priority
    }
    if (!IsFloorWary(ctx.floorAlert))
    {
        return 0.0f; // Below the threshold, ordinary Idle/Wander still apply
    }
    if (ctx.rooms.empty())
    {
        return 0.0f;
    }
    return 1.0f;
}

static void StartPatrol(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    self.moveSpeedScale = 0.7f;
    self.path.clear();
    self.pathIndex = 0;
    self.repathTimer = 0.0f;
    self.patrolRoomIndex = PickPatrolRoomIndex(ctx);
}

static BehaviorStatus UpdatePatrol(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    if (self.patrolRoomIndex < 0 || self.patrolRoomIndex >= (int)ctx.rooms.size())
    {
        StopEnemyMovement(self);
        return BEHAVIOR_FINISHED;
    }

    float goalX = (float)ctx.rooms[self.patrolRoomIndex].centerX();
    float goalY = (float)ctx.rooms[self.patrolRoomIndex].centerY();

    float dx = goalX - self.x;
    float dy = goalY - self.y;
    if (sqrtf(dx * dx + dy * dy) <= PATH_ARRIVE_DISTANCE * 3.0f)
    {
        StopEnemyMovement(self);
        self.patrolRoomIndex = -1; // Arrived, next pick chooses somewhere new (or re-joins a moving ally)
        return BEHAVIOR_FINISHED;
    }

    self.repathTimer -= ctx.dt;
    if (self.repathTimer <= 0.0f || self.path.empty())
    {
        RequestPath(ctx, goalX, goalY);
        float spread = (float)GetRandomValue(0, 100) / 100.0f;
        self.repathTimer = HUNT_REPATH_INTERVAL_MIN + spread * (HUNT_REPATH_INTERVAL_MAX - HUNT_REPATH_INTERVAL_MIN);
    }

    if (self.path.empty())
    {
        SetMoveToward(self, goalX, goalY, 0.7f);
        return BEHAVIOR_RUNNING;
    }

    FollowStoredPath(self, 0.7f);
    return BEHAVIOR_RUNNING;
}

// ---------- Registry ----------
// To add a behavior: write its score/start/update above, then add one row here.

std::vector<BehaviorDef> G_BEHAVIORS = {
    // id, name, max seconds, score, start, update
    { "idle",         "Idle",         0.0f, ScoreIdle,        StartIdle,        UpdateIdle },
    { "wander",       "Wander",       6.0f, ScoreWander,      StartWander,      UpdateWander },
    { "hunt",         "Hunt",         4.0f, ScoreHunt,        StartHunt,        UpdateHunt },
    { "melee_attack", "Melee Attack", 3.0f, ScoreMeleeAttack, StartMeleeAttack, UpdateMeleeAttack },
    { "follow_squad_order", "Follow Squad Order", 5.0f, ScoreSquadOrder, StartSquadOrder, UpdateSquadOrder },
    { "patrol",       "Patrol",       20.0f, ScorePatrol,      StartPatrol,      UpdatePatrol },
    { "flee",         "Flee",         3.0f, ScoreFlee,        StartFlee,        UpdateFlee }
};

const BehaviorDef* FindBehavior(const std::string& id)
{
    for (size_t i = 0; i < G_BEHAVIORS.size(); i++)
    {
        if (G_BEHAVIORS[i].id == id)
        {
            return &G_BEHAVIORS[i];
        }
    }
    return nullptr;
}