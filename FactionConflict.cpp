#include "FactionConflict.h"
#include "FactionData.h"
#include "EnemyFactory.h"
#include "EnemyData.h"
#include "raylib.h"
#include <map>

static std::string GetPrimaryFaction(const Enemy& enemy)
{
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr || archetype->factionIds.empty())
    {
        return "";
    }
    return archetype->factionIds[0];
}

static bool GroupsAreHostile(const std::string& factionA, const std::string& factionB)
{
    if (factionA.empty() || factionB.empty())
    {
        return false;
    }
    if (factionA == factionB)
    {
        return false; // Same primary faction, not a warring pair
    }
    return GetFactionStance({ factionA }, { factionB }) == STANCE_HOSTILE;
}

// Even matches read as a skirmish: a small mutual casualty chance regardless of side.
// The side that's badly outmatched bleeds far harder on top of that base chance.
static void ApplyCasualties(std::vector<Enemy*>& side, int sidePower, int opposingPower)
{
    if (sidePower <= 0 || opposingPower <= 0)
    {
        return;
    }

    float baseSkirmishChance = 0.15f;
    float disadvantage = ((float)opposingPower - (float)sidePower) / (float)opposingPower;
    if (disadvantage < 0.0f)
    {
        disadvantage = 0.0f;
    }
    float lossChance = baseSkirmishChance + (disadvantage * 0.6f);
    if (lossChance > 0.9f)
    {
        lossChance = 0.9f; // Never a guaranteed total wipe, something always has a chance to survive
    }

    for (size_t i = 0; i < side.size(); i++)
    {
        float roll = (float)GetRandomValue(0, 100) / 100.0f;
        if (roll < lossChance)
        {
            side[i]->hp = 0;
            side[i]->isDead = true;
        }
    }
}

void ResolveOffscreenFactionConflicts(std::vector<Enemy>& enemies, int floorNumber)
{
    std::map<std::string, std::vector<Enemy*>> groups;
    for (size_t i = 0; i < enemies.size(); i++)
    {
        if (enemies[i].isDead)
        {
            continue;
        }
        std::string faction = GetPrimaryFaction(enemies[i]);
        if (faction.empty())
        {
            continue;
        }
        groups[faction].push_back(&enemies[i]);
    }

    if (groups.size() < 2)
    {
        return; // Nothing to fight without at least two present factions
    }

    std::vector<std::string> factionIds;
    for (std::map<std::string, std::vector<Enemy*>>::iterator it = groups.begin(); it != groups.end(); ++it)
    {
        factionIds.push_back(it->first);
    }

    bool anyCasualty = false;

    for (size_t i = 0; i < factionIds.size(); i++)
    {
        for (size_t j = i + 1; j < factionIds.size(); j++)
        {
            if (!GroupsAreHostile(factionIds[i], factionIds[j]))
            {
                continue;
            }

            std::vector<Enemy*>& sideA = groups[factionIds[i]];
            std::vector<Enemy*>& sideB = groups[factionIds[j]];

            int powerA = 0;
            for (size_t a = 0; a < sideA.size(); a++)
            {
                powerA += sideA[a]->hp;
            }
            int powerB = 0;
            for (size_t b = 0; b < sideB.size(); b++)
            {
                powerB += sideB[b]->hp;
            }

            ApplyCasualties(sideA, powerA, powerB);
            ApplyCasualties(sideB, powerB, powerA);
            anyCasualty = true;
        }
    }

    if (!anyCasualty)
    {
        return;
    }

    // Scavengers move in on whatever's left unattended
    for (size_t i = 0; i < G_FACTIONS.size(); i++)
    {
        if (!G_FACTIONS[i].isScavenger)
        {
            continue;
        }

        bool alreadyPresent = groups.find(G_FACTIONS[i].id) != groups.end();
        if (alreadyPresent)
        {
            continue;
        }

        if (GetRandomValue(0, 100) > 40)
        {
            continue; // Not every corpse draws a scavenger
        }

        std::vector<const EnemyArchetype*> candidates;
        for (size_t a = 0; a < G_ENEMY_ARCHETYPES.size(); a++)
        {
            const EnemyArchetype& archetype = G_ENEMY_ARCHETYPES[a];
            if (archetype.factionIds.empty() || archetype.factionIds[0] != G_FACTIONS[i].id)
            {
                continue;
            }
            candidates.push_back(&archetype);
        }
        if (candidates.empty())
        {
            continue;
        }

        int corpseX = -1, corpseY = -1;
        for (size_t e = 0; e < enemies.size(); e++)
        {
            if (enemies[e].isDead)
            {
                corpseX = enemies[e].x;
                corpseY = enemies[e].y;
            }
        }
        if (corpseX < 0)
        {
            continue;
        }

        const EnemyArchetype* chosen = candidates[GetRandomValue(0, (int)candidates.size() - 1)];
        int spawnCount = GetRandomValue(1, 2);
        for (int s = 0; s < spawnCount; s++)
        {
            enemies.push_back(CreateEnemy(*chosen, floorNumber, corpseX, corpseY));
        }
    }
}
