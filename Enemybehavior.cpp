#include "EnemyBehavior.h"
#include "BrainData.h"
#include "EnemyFactory.h"
#include "FactionData.h"
#include "raylib.h"
#include <cmath>

static const float TARGET_KEEP_MULTIPLIER = 1.5f; // A target is only lost past this many detection radii. Acquiring it still needs line of sight.

static bool IsPlayerHostileTarget(const Enemy& actor)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(actor.archetypeId);
    if (archetype == nullptr)
    {
        return false;
    }
    std::vector<std::string> livingTag = { "living" };
    return GetFactionStance(archetype->factionIds, livingTag) == STANCE_HOSTILE;
}

static bool HasSightOf(const AIContext& ctx, float targetX, float targetY)
{
    int fromX = (int)(ctx.self.x + 0.5f);
    int fromY = (int)(ctx.self.y + 0.5f);
    int toX = (int)(targetX + 0.5f);
    int toY = (int)(targetY + 0.5f);
    return HasLineOfSight(fromX, fromY, toX, toY, ctx.isOpaque);
}

static float DistanceBetween(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

// ---------- Targeting ----------

static void AcquireTarget(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    float radius = (float)GetEnemyDetectionRadius(self);

    // Keep the current target while it stays reasonably close, even around a corner
    float keepX = 0.0f;
    float keepY = 0.0f;
    if (GetTargetPosition(ctx, keepX, keepY))
    {
        if (DistanceBetween(self.x, self.y, keepX, keepY) <= radius * TARGET_KEEP_MULTIPLIER)
        {
            return;
        }
    }

    self.targetIsPlayer = false;
    self.targetIndex = -1;
    float bestDistance = radius;

    if (IsPlayerHostileTarget(self))
    {
        float playerDistance = DistanceBetween(self.x, self.y, ctx.player.x, ctx.player.y);
        if (playerDistance <= bestDistance && HasSightOf(ctx, ctx.player.x, ctx.player.y))
        {
            self.targetIsPlayer = true;
            self.targetIndex = -1;
            bestDistance = playerDistance;
        }
    }

    for (size_t i = 0; i < ctx.enemies.size(); i++)
    {
        Enemy& other = ctx.enemies[i];
        if (&other == &self || other.isDead)
        {
            continue;
        }
        if (GetStance(self, other) != STANCE_HOSTILE)
        {
            continue;
        }
        float otherDistance = DistanceBetween(self.x, self.y, other.x, other.y);
        if (otherDistance < bestDistance && HasSightOf(ctx, other.x, other.y))
        {
            self.targetIsPlayer = false;
            self.targetIndex = (int)i;
            bestDistance = otherDistance;
        }
    }
}

// ---------- Selection ----------

static bool IsReflex(const BrainDef& brain, const std::string& behaviorId)
{
    for (size_t i = 0; i < brain.reflexIds.size(); i++)
    {
        if (brain.reflexIds[i] == behaviorId)
        {
            return true;
        }
    }
    return false;
}

static const BehaviorDef* FindFiringReflex(const AIContext& ctx, const BrainDef& brain)
{
    for (size_t i = 0; i < brain.reflexIds.size(); i++)
    {
        const BehaviorDef* def = FindBehavior(brain.reflexIds[i]);
        if (def == nullptr)
        {
            continue;
        }
        if (def->score(ctx) > 0.0f)
        {
            return def;
        }
    }
    return nullptr;
}

// finalWeight = brain weight * behavior context score. A zero score removes the behavior from the roll.
static const BehaviorDef* PickBehavior(const AIContext& ctx, const BrainDef& brain)
{
    std::vector<const BehaviorDef*> defs;
    std::vector<float> weights;
    float totalWeight = 0.0f;

    for (size_t i = 0; i < brain.entries.size(); i++)
    {
        const BehaviorDef* def = FindBehavior(brain.entries[i].behaviorId);
        float weight = 0.0f;
        if (def != nullptr)
        {
            float score = def->score(ctx);
            if (score > 0.0f)
            {
                weight = (float)brain.entries[i].baseWeight * score;
            }
        }
        defs.push_back(def);
        weights.push_back(weight);
        totalWeight += weight;
    }

    if (totalWeight <= 0.0f)
    {
        return nullptr;
    }

    float roll = ((float)GetRandomValue(0, 9999) / 10000.0f) * totalWeight;
    const BehaviorDef* lastPositive = nullptr;
    for (size_t i = 0; i < defs.size(); i++)
    {
        if (weights[i] <= 0.0f)
        {
            continue;
        }
        lastPositive = defs[i];
        if (roll < weights[i])
        {
            return defs[i];
        }
        roll -= weights[i];
    }
    return lastPositive;
}

static void StartBehavior(const AIContext& ctx, const BehaviorDef& def)
{
    Enemy& self = ctx.self;
    self.behaviorId = def.id;
    self.behaviorTimer = 0.0f;
    self.behaviorLimit = def.maxDuration;
    self.hasGoal = false;
    self.moveSpeedScale = 1.0f;
    def.start(ctx);
}

// ---------- Movement ----------

// Refuses a step only if it squeezes closer to another body that is already too near. Bodies that
// start overlapped (scavengers spawn on a corpse tile) can still walk apart.
static bool WouldCrowd(const Enemy& self, float newX, float newY, float otherX, float otherY)
{
    float spacingSq = ENEMY_BODY_SPACING * ENEMY_BODY_SPACING;
    float newDx = otherX - newX;
    float newDy = otherY - newY;
    float newDistSq = newDx * newDx + newDy * newDy;
    if (newDistSq >= spacingSq)
    {
        return false;
    }
    float oldDx = otherX - self.x;
    float oldDy = otherY - self.y;
    float oldDistSq = oldDx * oldDx + oldDy * oldDy;
    if (newDistSq < oldDistSq)
    {
        return true;
    }
    return false;
}

static bool IsStepBlockedByBodies(const AIContext& ctx, float newX, float newY)
{
    Enemy& self = ctx.self;
    if (WouldCrowd(self, newX, newY, ctx.player.x, ctx.player.y))
    {
        return true;
    }
    for (size_t i = 0; i < ctx.enemies.size(); i++)
    {
        const Enemy& other = ctx.enemies[i];
        if (&other == &self || other.isDead)
        {
            continue;
        }
        if (WouldCrowd(self, newX, newY, other.x, other.y))
        {
            return true;
        }
    }
    return false;
}

static void ApplyMovement(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    if (self.moveDirX == 0.0f && self.moveDirY == 0.0f)
    {
        return;
    }

    float speed = ENEMY_BASE_MOVE_SPEED * ((float)GetEnemySpeed(self) / 100.0f) * self.moveSpeedScale;
    float step = speed * ctx.dt;

    // Each axis on its own so a wall slides the enemy instead of stopping it, same as the player
    float candidateX = self.x + self.moveDirX * step;
    if (ctx.isPositionFree(candidateX + 0.5f, self.y + 0.5f, ENEMY_COLLISION_RADIUS) && !IsStepBlockedByBodies(ctx, candidateX, self.y))
    {
        self.x = candidateX;
    }

    float candidateY = self.y + self.moveDirY * step;
    if (ctx.isPositionFree(self.x + 0.5f, candidateY + 0.5f, ENEMY_COLLISION_RADIUS) && !IsStepBlockedByBodies(ctx, self.x, candidateY))
    {
        self.y = candidateY;
    }
}

// ---------- Per-enemy update ----------

static void UpdateOneEnemy(const AIContext& ctx)
{
    Enemy& self = ctx.self;
    const BrainDef* brain = GetBrainForEnemy(self);
    if (brain == nullptr)
    {
        return;
    }

    if (self.lastHp < 0)
    {
        self.lastHp = self.hp;
    }
    if (self.attackCooldown > 0.0f)
    {
        self.attackCooldown -= ctx.dt;
    }

    AcquireTarget(ctx);

    // Interrupts: got hurt, or a hostile target appeared or vanished
    bool hasThreat = EnemyHasTarget(self);
    bool interrupted = false;
    if (self.hp < self.lastHp)
    {
        interrupted = true;
    }
    if (hasThreat != self.hadThreat)
    {
        interrupted = true;
    }
    self.lastHp = self.hp;
    self.hadThreat = hasThreat;

    const BehaviorDef* current = FindBehavior(self.behaviorId);
    bool needsPick = false;
    if (current == nullptr || interrupted)
    {
        needsPick = true;
    }

    // Reflexes override the roll. One already running is left alone, not restarted every frame.
    const BehaviorDef* reflex = FindFiringReflex(ctx, *brain);
    if (reflex != nullptr)
    {
        if (self.behaviorId != reflex->id)
        {
            StartBehavior(ctx, *reflex);
        }
        current = reflex;
        needsPick = false;
    }
    else if (current != nullptr && IsReflex(*brain, current->id))
    {
        needsPick = true; // The reflex condition cleared, go back to normal life
    }

    if (needsPick)
    {
        const BehaviorDef* picked = PickBehavior(ctx, *brain);
        if (picked != nullptr)
        {
            StartBehavior(ctx, *picked);
            current = picked;
        }
        else
        {
            self.behaviorId = "";
            StopEnemyMovement(self);
            current = nullptr;
        }
    }

    if (current != nullptr)
    {
        self.behaviorTimer += ctx.dt;
        BehaviorStatus status = current->update(ctx);

        bool timedOut = false;
        if (self.behaviorLimit > 0.0f && self.behaviorTimer >= self.behaviorLimit)
        {
            timedOut = true;
        }
        if (status == BEHAVIOR_FINISHED || timedOut)
        {
            self.behaviorId = ""; // Next frame rolls a new one
            StopEnemyMovement(self);
        }
    }

    ApplyMovement(ctx);
}

void UpdateEnemyBehaviors(std::vector<Enemy>& enemies, Player& player, float dt, const PositionFreeFn& isPositionFree, const TileOpaqueFn& isOpaque, const NavMesh& navMesh, std::string& actionMessage)
{
    for (size_t i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].isDead)
        {
            continue;
        }
        AIContext ctx = { enemies[i], enemies, player, dt, isPositionFree, isOpaque, navMesh, actionMessage };
        UpdateOneEnemy(ctx);
    }
}