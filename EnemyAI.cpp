#include "EnemyAI.h"
#include "EnemyFactory.h"
#include "FactionData.h"
#include "raylib.h"

static const int DEFAULT_DETECTION_RADIUS = 8; // Fallback if an enemy's archetype can't be found
static const int ALLY_CHECK_RADIUS = 4;
static const int TERRITORIAL_LEASH_RADIUS = 6;
static const float FLEE_HEALTH_RATIO = 0.3f;

static int AbsInt(int value)
{
    if (value < 0)
    {
        return -value;
    }
    return value;
}

static int MaxInt(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    return b;
}

static int ChebyshevDistance(int x1, int y1, int x2, int y2)
{
    return MaxInt(AbsInt(x1 - x2), AbsInt(y1 - y2));
}

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

struct ThreatInfo
{
    bool found = false;
    bool isPlayer = false;
    Enemy* enemy = nullptr;
    int x = 0;
    int y = 0;
    int distance = 0;
};

static ThreatInfo FindNearestHostileThreat(Enemy& actor, std::vector<Enemy>& enemies, Player& player, int radius, const TileOpaqueFn& isOpaque)
{
    ThreatInfo best;
    int bestDist = radius + 1;

    if (IsPlayerHostileTarget(actor))
    {
        int dist = ChebyshevDistance(actor.x, actor.y, player.x, player.y);
        if (dist <= radius && dist < bestDist && HasLineOfSight(actor.x, actor.y, player.x, player.y, isOpaque))
        {
            best.found = true;
            best.isPlayer = true;
            best.x = player.x;
            best.y = player.y;
            best.distance = dist;
            bestDist = dist;
        }
    }

    for (size_t i = 0; i < enemies.size(); i++)
    {
        Enemy& other = enemies[i];
        if (&other == &actor || other.isDead)
        {
            continue;
        }
        if (GetStance(actor, other) != STANCE_HOSTILE)
        {
            continue;
        }
        int dist = ChebyshevDistance(actor.x, actor.y, other.x, other.y);
        if (dist <= radius && dist < bestDist && HasLineOfSight(actor.x, actor.y, other.x, other.y, isOpaque))
        {
            best.found = true;
            best.isPlayer = false;
            best.enemy = &other;
            best.x = other.x;
            best.y = other.y;
            best.distance = dist;
            bestDist = dist;
        }
    }

    return best;
}

static int CountAlliesNearby(Enemy& actor, std::vector<Enemy>& enemies, int radius)
{
    int count = 0;
    for (size_t i = 0; i < enemies.size(); i++)
    {
        Enemy& other = enemies[i];
        if (&other == &actor || other.isDead)
        {
            continue;
        }
        if (GetStance(actor, other) != STANCE_ALLIED)
        {
            continue;
        }
        if (ChebyshevDistance(actor.x, actor.y, other.x, other.y) <= radius)
        {
            count++;
        }
    }
    return count;
}

static AIAction StepToward(Enemy& actor, int tx, int ty, const TileWalkableFn& isWalkable)
{
    int dx = 0;
    if (tx > actor.x)
    {
        dx = 1;
    }
    else if (tx < actor.x)
    {
        dx = -1;
    }
    int dy = 0;
    if (ty > actor.y)
    {
        dy = 1;
    }
    else if (ty < actor.y)
    {
        dy = -1;
    }

    AIAction action;
    action.type = AI_ACTION_MOVE;

    if (dx != 0 && dy != 0 && isWalkable(actor.x + dx, actor.y + dy))
    {
        action.moveX = dx;
        action.moveY = dy;
        return action;
    }
    if (dx != 0 && isWalkable(actor.x + dx, actor.y))
    {
        action.moveX = dx;
        action.moveY = 0;
        return action;
    }
    if (dy != 0 && isWalkable(actor.x, actor.y + dy))
    {
        action.moveX = 0;
        action.moveY = dy;
        return action;
    }

    action.type = AI_ACTION_IDLE;
    return action;
}

static AIAction StepAway(Enemy& actor, int fx, int fy, const TileWalkableFn& isWalkable)
{
    int dx = 0;
    if (fx > actor.x)
    {
        dx = -1;
    }
    else if (fx < actor.x)
    {
        dx = 1;
    }
    int dy = 0;
    if (fy > actor.y)
    {
        dy = -1;
    }
    else if (fy < actor.y)
    {
        dy = 1;
    }

    AIAction action;
    action.type = AI_ACTION_MOVE;

    if (dx != 0 && dy != 0 && isWalkable(actor.x + dx, actor.y + dy))
    {
        action.moveX = dx;
        action.moveY = dy;
        return action;
    }
    if (dx != 0 && isWalkable(actor.x + dx, actor.y))
    {
        action.moveX = dx;
        action.moveY = 0;
        return action;
    }
    if (dy != 0 && isWalkable(actor.x, actor.y + dy))
    {
        action.moveX = 0;
        action.moveY = dy;
        return action;
    }

    action.type = AI_ACTION_IDLE;
    return action;
}

static AIAction AttackThreat(const ThreatInfo& threat)
{
    AIAction action;
    if (threat.isPlayer)
    {
        action.type = AI_ACTION_ATTACK_PLAYER;
    }
    else
    {
        action.type = AI_ACTION_ATTACK_ENEMY;
        action.targetEnemy = threat.enemy;
    }
    return action;
}

static AIAction DecideMindless(Enemy& actor, std::vector<Enemy>& enemies, Player& player, const TileWalkableFn& isWalkable, int detectionRadius, const TileOpaqueFn& isOpaque)
{
    ThreatInfo threat = FindNearestHostileThreat(actor, enemies, player, detectionRadius, isOpaque);
    if (!threat.found)
    {
        AIAction idle;
        return idle;
    }
    if (threat.distance <= 1)
    {
        return AttackThreat(threat);
    }
    return StepToward(actor, threat.x, threat.y, isWalkable);
}

static AIAction DecidePredator(Enemy& actor, std::vector<Enemy>& enemies, Player& player, const TileWalkableFn& isWalkable, int detectionRadius, const TileOpaqueFn& isOpaque)
{
    float healthRatio = 1.0f;
    if (actor.maxHp > 0)
    {
        healthRatio = (float)actor.hp / (float)actor.maxHp;
    }
    int allyCount = CountAlliesNearby(actor, enemies, ALLY_CHECK_RADIUS);

    ThreatInfo threat = FindNearestHostileThreat(actor, enemies, player, detectionRadius, isOpaque);

    if (healthRatio <= FLEE_HEALTH_RATIO && allyCount == 0 && threat.found)
    {
        return StepAway(actor, threat.x, threat.y, isWalkable);
    }

    if (!threat.found)
    {
        AIAction idle;
        return idle;
    }
    if (threat.distance <= 1)
    {
        return AttackThreat(threat);
    }
    return StepToward(actor, threat.x, threat.y, isWalkable);
}

static AIAction DecideSkittish(Enemy& actor, std::vector<Enemy>& enemies, Player& player, const TileWalkableFn& isWalkable, int detectionRadius, const TileOpaqueFn& isOpaque)
{
    ThreatInfo threat = FindNearestHostileThreat(actor, enemies, player, detectionRadius, isOpaque);

    bool playerTooClose = ChebyshevDistance(actor.x, actor.y, player.x, player.y) <= 2
        && HasLineOfSight(actor.x, actor.y, player.x, player.y, isOpaque);
    if (!threat.found && playerTooClose)
    {
        threat.found = true;
        threat.isPlayer = true;
        threat.x = player.x;
        threat.y = player.y;
        threat.distance = ChebyshevDistance(actor.x, actor.y, player.x, player.y);
    }

    if (!threat.found)
    {
        AIAction idle;
        return idle;
    }

    if (threat.distance <= 1)
    {
        AIAction flee = StepAway(actor, threat.x, threat.y, isWalkable);
        if (flee.type == AI_ACTION_IDLE)
        {
            return AttackThreat(threat); // Cornered, nowhere to run
        }
        return flee;
    }

    return StepAway(actor, threat.x, threat.y, isWalkable);
}

static AIAction DecideTerritorial(Enemy& actor, int homeX, int homeY, std::vector<Enemy>& enemies, Player& player, const TileWalkableFn& isWalkable, int detectionRadius, const TileOpaqueFn& isOpaque)
{
    ThreatInfo threat = FindNearestHostileThreat(actor, enemies, player, detectionRadius, isOpaque);

    if (threat.found)
    {
        int homeDistToThreat = ChebyshevDistance(homeX, homeY, threat.x, threat.y);
        if (homeDistToThreat <= TERRITORIAL_LEASH_RADIUS)
        {
            if (threat.distance <= 1)
            {
                return AttackThreat(threat);
            }
            return StepToward(actor, threat.x, threat.y, isWalkable);
        }
    }

    if (ChebyshevDistance(actor.x, actor.y, homeX, homeY) > 0)
    {
        return StepToward(actor, homeX, homeY, isWalkable);
    }

    AIAction idle;
    return idle;
}

static AIAction DecideCaster(Enemy& actor, std::vector<Enemy>& enemies, Player& player, const TileWalkableFn& isWalkable, int detectionRadius, const TileOpaqueFn& isOpaque)
{
    ThreatInfo threat = FindNearestHostileThreat(actor, enemies, player, detectionRadius, isOpaque);
    if (!threat.found)
    {
        AIAction idle;
        return idle;
    }

    const int preferredDistance = 3;
    if (threat.distance <= 1)
    {
        return StepAway(actor, threat.x, threat.y, isWalkable); // No ranged attack yet, back off instead of trading blows
    }
    if (threat.distance < preferredDistance)
    {
        return StepAway(actor, threat.x, threat.y, isWalkable);
    }
    if (threat.distance > preferredDistance)
    {
        return StepToward(actor, threat.x, threat.y, isWalkable);
    }

    AIAction idle;
    return idle; // Holding at preferred range, would cast here once spells exist
}

AIAction DecideEnemyAction(Enemy& actor, int homeX, int homeY, std::vector<Enemy>& enemies, Player& player, const TileWalkableFn& isWalkable, const TileOpaqueFn& isOpaque)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(actor.archetypeId);
    AIBrain brain = BRAIN_MINDLESS;
    if (archetype != nullptr)
    {
        brain = archetype->brain;
    }
    int detectionRadius = GetEnemyDetectionRadius(actor);

    switch (brain)
    {
    case BRAIN_PREDATOR:
        return DecidePredator(actor, enemies, player, isWalkable, detectionRadius, isOpaque);
    case BRAIN_SKITTISH:
        return DecideSkittish(actor, enemies, player, isWalkable, detectionRadius, isOpaque);
    case BRAIN_TERRITORIAL:
        return DecideTerritorial(actor, homeX, homeY, enemies, player, isWalkable, detectionRadius, isOpaque);
    case BRAIN_CASTER:
        return DecideCaster(actor, enemies, player, isWalkable, detectionRadius, isOpaque);
    case BRAIN_MINDLESS:
    default:
        return DecideMindless(actor, enemies, player, isWalkable, detectionRadius, isOpaque);
    }
}

int CalculateEnemyAttackDamage(const Enemy& attacker, int defenderArmor)
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
