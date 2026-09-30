#include "FloorPopulation.h"
#include "EnemyFactory.h"
#include "FactionData.h"
#include "BrainData.h"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <string>

const int ENEMY_BUDGET_PER_FLOOR = 40; // A group that would push the floor past this is skipped
const int ORGANIZATION_FLOOR_NUDGE = 1; // Per floor into the chunk, added to the squad and warband weights
const int GROUP_SLOT_COUNT = 9;

// Odds of each organization level by chunk index. Columns: stragglers, packs, squads, warbands.
// Chunks past the last row reuse the last row. To retune pacing, change numbers here.
static const std::vector<std::vector<int>> G_ORGANIZATION_BY_CHUNK = {
    { 70, 25, 5, 0 },
    { 30, 40, 25, 5 },
    { 10, 30, 40, 20 },
    { 5, 15, 40, 40 }
};

// Group members stand on these tiles around the room center. Slot 0 (the center) is the leader.
// Rooms are at least 4x4, so every offset stays inside the room.
static const int GROUP_SLOT_OFFSETS[GROUP_SLOT_COUNT][2] = {
    { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { -1, -1 }, { 1, -1 }, { -1, 1 }
};

static int RollBaseOrganizationLevel(const FloorParams& params)
{
    int row = params.chunkIndex;
    if (row >= (int)G_ORGANIZATION_BY_CHUNK.size())
    {
        row = (int)G_ORGANIZATION_BY_CHUNK.size() - 1;
    }

    std::vector<int> weights = G_ORGANIZATION_BY_CHUNK[row];
    int nudge = (params.floorInChunk - 1) * ORGANIZATION_FLOOR_NUDGE;
    weights[2] += nudge;
    weights[3] += nudge;

    int totalWeight = 0;
    for (size_t i = 0; i < weights.size(); i++)
    {
        totalWeight += weights[i];
    }

    int roll = GetRandomValue(1, totalWeight);
    for (size_t i = 0; i < weights.size(); i++)
    {
        if (roll <= weights[i])
        {
            return (int)i;
        }
        roll -= weights[i];
    }
    return 0;
}

const int MAX_ORGANIZATION_LEVEL = 3;

// The chunk's rolled level shifted by any active modifiers, kept inside 0 to 3.
static int RollOrganizationLevel(const FloorParams& params)
{
    int level = RollBaseOrganizationLevel(params) + params.organizationBonus;
    if (level < 0)
    {
        level = 0;
    }
    if (level > MAX_ORGANIZATION_LEVEL)
    {
        level = MAX_ORGANIZATION_LEVEL;
    }
    return level;
}

static int RollGroupSize(int level)
{
    if (level <= 0)
    {
        return 1;
    }
    if (level == 1)
    {
        return GetRandomValue(2, 3);
    }
    if (level == 2)
    {
        return GetRandomValue(3, 5);
    }
    return GetRandomValue(4, 6);
}

static bool SharesAnyTag(const std::vector<std::string>& listA, const std::vector<std::string>& listB)
{
    for (size_t a = 0; a < listA.size(); a++)
    {
        for (size_t b = 0; b < listB.size(); b++)
        {
            if (listA[a] == listB[b])
            {
                return true;
            }
        }
    }
    return false;
}

// A group mate drawn from the same weighted pool as the anchor, so a themed floor stays mostly on theme.
// Rejects anything hostile to the anchor, or the group would fight itself. Falls back to the anchor.
static const EnemyArchetype* PickCompanion(const FloorParams& params, const EnemyArchetype& anchor)
{
    for (int attempt = 0; attempt < 8; attempt++)
    {
        const EnemyArchetype* candidate = PickSpawnArchetype(params);
        if (candidate == nullptr)
        {
            continue;
        }
        if (GetFactionStance(anchor.factionIds, candidate->factionIds) == STANCE_HOSTILE)
        {
            continue;
        }
        if (GetFactionStance(candidate->factionIds, anchor.factionIds) == STANCE_HOSTILE)
        {
            continue;
        }
        return candidate;
    }
    return &anchor;
}

static bool IsMinorityCandidate(const EnemyArchetype& archetype, const ExpeditionDef& expedition)
{
    if (!expedition.minorTags.empty())
    {
        return SharesAnyTag(archetype.factionIds, expedition.minorTags);
    }
    return GetFactionStance(archetype.factionIds, expedition.themeTags) == STANCE_HOSTILE;
}

// Weighted by the plain spawn rules (no theme scaling), so depth still decides what can show up.
static const EnemyArchetype* PickMinorityArchetype(const FloorParams& params)
{
    std::vector<const EnemyArchetype*> candidates;
    std::vector<int> weights;
    int totalWeight = 0;

    for (size_t i = 0; i < G_SPAWN_RULES.size(); i++)
    {
        int weight = CalculateSpawnWeight(G_SPAWN_RULES[i], params.floorNumber);
        if (weight <= 0)
        {
            continue;
        }
        const EnemyArchetype* archetype = FindEnemyArchetype(G_SPAWN_RULES[i].archetype_id);
        if (archetype == nullptr)
        {
            continue;
        }
        if (!IsMinorityCandidate(*archetype, *params.expedition))
        {
            continue;
        }
        candidates.push_back(archetype);
        weights.push_back(weight);
        totalWeight += weight;
    }

    if (totalWeight <= 0)
    {
        return nullptr;
    }

    int roll = GetRandomValue(1, totalWeight);
    for (size_t i = 0; i < candidates.size(); i++)
    {
        if (roll <= weights[i])
        {
            return candidates[i];
        }
        roll -= weights[i];
    }
    return candidates.back();
}

// The room farthest from BOTH stairs, so the player has to go looking for it.
static int PickHiddenRoom(const std::vector<Room>& rooms, int downStairRoomIndex)
{
    if (rooms.size() < 3 || downStairRoomIndex < 0 || downStairRoomIndex >= (int)rooms.size())
    {
        return -1;
    }

    int bestIndex = -1;
    float bestScore = -1.0f;
    for (size_t i = 1; i < rooms.size(); i++)
    {
        if ((int)i == downStairRoomIndex)
        {
            continue;
        }
        float upDx = (float)(rooms[i].centerX() - rooms[0].centerX());
        float upDy = (float)(rooms[i].centerY() - rooms[0].centerY());
        float downDx = (float)(rooms[i].centerX() - rooms[downStairRoomIndex].centerX());
        float downDy = (float)(rooms[i].centerY() - rooms[downStairRoomIndex].centerY());
        float upDistance = sqrtf(upDx * upDx + upDy * upDy);
        float downDistance = sqrtf(downDx * downDx + downDy * downDy);
        float score = std::min(upDistance, downDistance);
        if (score > bestScore)
        {
            bestScore = score;
            bestIndex = (int)i;
        }
    }
    return bestIndex;
}

// mixedMembers true: companions come from the floor's weighted pool. False: every member is the anchor's kind.
static void PlaceGroup(const FloorParams& params, const Room& room, const EnemyArchetype& anchor, int level, int size, int groupId, bool mixedMembers, std::vector<Enemy>& enemies)
{
    if (size > GROUP_SLOT_COUNT)
    {
        size = GROUP_SLOT_COUNT;
    }

    for (int i = 0; i < size; i++)
    {
        const EnemyArchetype* archetype = &anchor;
        if (i > 0 && mixedMembers)
        {
            archetype = PickCompanion(params, anchor);
        }

        // Warbands are led by a lord or master
        std::string forcedVariant = "";
        if (level >= 3 && i == 0)
        {
            if (GetRandomValue(0, 1) == 0)
            {
                forcedVariant = "lord";
            }
            else
            {
                forcedVariant = "master";
            }
        }

        int tileX = room.centerX() + GROUP_SLOT_OFFSETS[i][0];
        int tileY = room.centerY() + GROUP_SLOT_OFFSETS[i][1];
        Enemy enemy = CreateEnemy(*archetype, params.floorNumber, tileX, tileY, forcedVariant, params.variantChanceBonus);
        enemy.detectionBonus = params.detectionRadiusMod;

        if (level >= 1)
        {
            enemy.groupId = groupId;
        }

        // Squads and up coordinate, except things that are mindless by nature. Those keep hunting on their own.
        if (level >= 2)
        {
            const BrainDef* brain = GetBrainForEnemy(enemy);
            if (brain != nullptr && brain->id != "mindless")
            {
                enemy.brainOverrideId = "squad_martial";
            }
        }

        enemies.push_back(enemy);
    }
}

void PopulateFloor(const FloorParams& params, const std::vector<Room>& rooms, int downStairRoomIndex, std::vector<Enemy>& enemies)
{
    enemies.clear();
    if (rooms.size() < 2)
    {
        return;
    }

    int nextGroupId = 0;

    // Hidden minority first, so the budget never squeezes it out
    int hiddenRoom = -1;
    if (params.expedition != nullptr && rooms.size() >= 4)
    {
        int chance = (int)(params.expedition->minorGroupChance * params.minorGroupChanceMult * 100.0f);
        if (GetRandomValue(1, 100) <= chance)
        {
            hiddenRoom = PickHiddenRoom(rooms, downStairRoomIndex);
        }
    }
    if (hiddenRoom >= 0)
    {
        const EnemyArchetype* minority = PickMinorityArchetype(params);
        if (minority == nullptr)
        {
            hiddenRoom = -1; // Nothing hostile can spawn at this depth, the room gets a normal group
        }
        else
        {
            int level = RollOrganizationLevel(params);
            if (level < 1)
            {
                level = 1; // A hidden group is never a lone straggler
            }
            PlaceGroup(params, rooms[hiddenRoom], *minority, level, RollGroupSize(level), nextGroupId, false, enemies);
            nextGroupId++;
        }
    }

    // Start at a random room so the budget does not always leave the same end of the map empty
    int roomCount = (int)rooms.size() - 1;
    int startOffset = GetRandomValue(0, roomCount - 1);
    for (int step = 0; step < roomCount; step++)
    {
        int roomIndex = 1 + ((startOffset + step) % roomCount);
        if (roomIndex == hiddenRoom)
        {
            continue;
        }
        if (roomIndex == downStairRoomIndex && params.floorInChunk == FLOORS_PER_CHUNK)
        {
            continue; // The exit room on an extraction floor stays empty, so a death there is never the stairs' fault
        }

        const EnemyArchetype* anchor = PickSpawnArchetype(params);
        if (anchor == nullptr)
        {
            continue;
        }

        int level = RollOrganizationLevel(params);
        int size = RollGroupSize(level);
        if ((int)enemies.size() + size > ENEMY_BUDGET_PER_FLOOR)
        {
            continue;
        }

        PlaceGroup(params, rooms[roomIndex], *anchor, level, size, nextGroupId, true, enemies);
        nextGroupId++;
    }
}