#include "Squad.h"
#include "BehaviorData.h"
#include "EnemyFactory.h"
#include "BrainData.h"
#include "raylib.h"
#include <cmath>
#include <map>
#include <string>

static const float SQUAD_CLUSTER_RADIUS = 7.0f;   // Same-faction, same-target enemies within this distance (or chained through others this close) count as one squad
static const float FRONTLINE_DISTANCE = ENEMY_MELEE_RANGE * 0.6f; // Comfortably inside melee range so MeleeAttack reliably takes over once formed up
static const float BACKLINE_DISTANCE = 3.0f;
static const float FRONTLINE_ARC_SPACING_DEG = 24.0f;
static const float BACKLINE_ARC_SPACING_DEG = 18.0f;
static const float SKIRMISHER_FLANK_ANGLE_DEG = 75.0f; // Base angle off the squad's main approach direction. Left one gap open rather than a full ring, so a surround is escapable.

static std::string GetPrimaryFactionId(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr || archetype->factionIds.empty())
    {
        return "";
    }
    return archetype->factionIds[0];
}

static bool CanShareSquad(const std::vector<Enemy>& enemies, int indexA, int indexB)
{
    const Enemy& a = enemies[indexA];
    const Enemy& b = enemies[indexB];

    if (a.targetIsPlayer != b.targetIsPlayer)
    {
        return false;
    }
    if (!a.targetIsPlayer && a.targetIndex != b.targetIndex)
    {
        return false;
    }

    bool sameGroup = (a.groupId >= 0 && a.groupId == b.groupId);
    if (!sameGroup)
    {
        std::string factionA = GetPrimaryFactionId(a);
        if (factionA.empty() || factionA != GetPrimaryFactionId(b))
        {
            return false;
        }
    }

    float dx = a.x - b.x;
    float dy = a.y - b.y;
    if (sqrtf(dx * dx + dy * dy) > SQUAD_CLUSTER_RADIUS)
    {
        return false;
    }
    return true;
}

static int FindSquadRoot(std::vector<int>& parent, int node)
{
    while (parent[node] != node)
    {
        parent[node] = parent[parent[node]];
        node = parent[node];
    }
    return node;
}

// Rotates (x, y) by angleDeg around the origin. Used to turn the squad's approach direction into
// an arc offset or a flank direction.
static void RotateVector(float x, float y, float angleDeg, float& outX, float& outY)
{
    float rad = angleDeg * DEG2RAD;
    float cosA = cosf(rad);
    float sinA = sinf(rad);
    outX = x * cosA - y * sinA;
    outY = x * sinA + y * cosA;
}

// Spreads members evenly across an arc of the given radius around (targetX, targetY), centered on
// the approach direction. Used for both the frontline shield wall and the backline huddle, just
// with different distance and spacing.
static void PlaceArc(std::vector<Enemy>& enemies, const std::vector<int>& members, float targetX, float targetY, float approachX, float approachY, float distance, float spacingDeg)
{
    for (size_t i = 0; i < members.size(); i++)
    {
        float offsetDeg = ((float)i - ((float)members.size() - 1.0f) * 0.5f) * spacingDeg;
        float dirX = 0.0f;
        float dirY = 0.0f;
        RotateVector(approachX, approachY, offsetDeg, dirX, dirY);

        Enemy& member = enemies[members[i]];
        member.hasSquadSlot = true;
        member.squadSlotX = targetX + dirX * distance;
        member.squadSlotY = targetY + dirY * distance;
    }
}

static void PlaceSkirmisherFlanks(std::vector<Enemy>& enemies, const std::vector<int>& members, float targetX, float targetY, float approachX, float approachY)
{
    for (size_t i = 0; i < members.size(); i++)
    {
        float angleMagnitude = SKIRMISHER_FLANK_ANGLE_DEG + (float)(i / 2) * 10.0f;
        float angle = angleMagnitude;
        if (i % 2 == 1)
        {
            angle = -angleMagnitude;
        }

        float dirX = 0.0f;
        float dirY = 0.0f;
        RotateVector(approachX, approachY, angle, dirX, dirY);

        Enemy& member = enemies[members[i]];
        member.hasSquadSlot = true;
        member.squadSlotX = targetX + dirX * FRONTLINE_DISTANCE;
        member.squadSlotY = targetY + dirY * FRONTLINE_DISTANCE;
    }
}

static void ComputeClusterFormation(std::vector<Enemy>& enemies, const std::vector<int>& members, float targetX, float targetY)
{
    float centroidX = 0.0f;
    float centroidY = 0.0f;
    for (int index : members)
    {
        centroidX += enemies[index].x;
        centroidY += enemies[index].y;
    }
    centroidX /= (float)members.size();
    centroidY /= (float)members.size();

    // Direction from the target out to the squad. Frontline forms up facing back along this line.
    float approachX = centroidX - targetX;
    float approachY = centroidY - targetY;
    float approachLength = sqrtf(approachX * approachX + approachY * approachY);
    if (approachLength < 0.001f)
    {
        approachX = 1.0f;
        approachY = 0.0f;
        approachLength = 1.0f;
    }
    approachX /= approachLength;
    approachY /= approachLength;

    std::vector<int> frontline;
    std::vector<int> skirmishers;
    std::vector<int> backline;
    for (int index : members)
    {
        EnemyRole role = ROLE_FRONTLINE;
        const EnemyArchetype* archetype = FindEnemyArchetype(enemies[index].archetypeId);
        if (archetype != nullptr)
        {
            role = archetype->role;
        }

        if (role == ROLE_BACKLINE)
        {
            backline.push_back(index);
        }
        else if (role == ROLE_SKIRMISHER)
        {
            skirmishers.push_back(index);
        }
        else
        {
            frontline.push_back(index);
        }
    }

    PlaceArc(enemies, frontline, targetX, targetY, approachX, approachY, FRONTLINE_DISTANCE, FRONTLINE_ARC_SPACING_DEG);
    PlaceArc(enemies, backline, targetX, targetY, approachX, approachY, BACKLINE_DISTANCE, BACKLINE_ARC_SPACING_DEG);
    PlaceSkirmisherFlanks(enemies, skirmishers, targetX, targetY, approachX, approachY);
}

// Only brains that list follow_squad_order can walk to a formation slot. Anyone else holding a slot has
// Hunt scored to zero and just stands there, so they stay out of squads and hunt on their own.
static bool CanFollowSquadOrders(const Enemy& enemy)
{
    const BrainDef* brain = GetBrainForEnemy(enemy);
    if (brain == nullptr)
    {
        return false;
    }
    for (size_t i = 0; i < brain->entries.size(); i++)
    {
        if (brain->entries[i].behaviorId == "follow_squad_order")
        {
            return true;
        }
    }
    return false;
}

void UpdateSquads(std::vector<Enemy>& enemies, Player& player)
{
    for (Enemy& enemy : enemies)
    {
        enemy.hasSquadSlot = false;
    }

    std::vector<int> active;
    for (size_t i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].isDead)
        {
            continue;
        }
        if (!EnemyHasTarget(enemies[i]))
        {
            continue;
        }
        if (!CanFollowSquadOrders(enemies[i]))
        {
            continue;
        }
        active.push_back((int)i);
    }

    if (active.size() < 2)
    {
        return; // Nobody to squad up with
    }

    std::vector<int> parent(active.size());
    for (size_t i = 0; i < parent.size(); i++)
    {
        parent[i] = (int)i;
    }

    for (size_t a = 0; a < active.size(); a++)
    {
        for (size_t b = a + 1; b < active.size(); b++)
        {
            if (!CanShareSquad(enemies, active[a], active[b]))
            {
                continue;
            }
            int rootA = FindSquadRoot(parent, (int)a);
            int rootB = FindSquadRoot(parent, (int)b);
            if (rootA != rootB)
            {
                parent[rootA] = rootB;
            }
        }
    }

    std::map<int, std::vector<int>> clusters;
    for (size_t i = 0; i < active.size(); i++)
    {
        int root = FindSquadRoot(parent, (int)i);
        clusters[root].push_back(active[i]);
    }

    for (auto& clusterPair : clusters)
    {
        std::vector<int>& members = clusterPair.second;
        if (members.size() < 2)
        {
            continue; // No one nearby to coordinate with, plain Hunt handles it
        }

        float targetX = 0.0f;
        float targetY = 0.0f;
        if (!GetTargetPosition(enemies[members[0]], enemies, player, targetX, targetY))
        {
            continue;
        }

        ComputeClusterFormation(enemies, members, targetX, targetY);
    }
}