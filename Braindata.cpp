#include "BrainData.h"
#include "EnemyData.h"
#include "EnemyFactory.h"

// To add a brain: add one row. To change a creature's personality: change its weights.
// Weights are relative to each other, only the ratios matter.
std::vector<BrainDef> G_BRAINS = {
    // Relentless. Undead, constructs. No real awareness to search with, so alert never gives them
    // Patrol: they idle or shamble until something is directly in front of them, always.
    { "mindless",
        { { "idle", 10 }, { "wander", 5 }, { "hunt", 100 }, { "melee_attack", 100 } },
        {} },

        // Hunts hard, breaks off when badly hurt.
        { "predator",
            { { "idle", 10 }, { "wander", 25 }, { "hunt", 100 }, { "melee_attack", 100 }, { "patrol", 60 } },
            { "flee" } },

            // Wanders and loiters, only bites what is already in reach, runs when hurt.
            { "skittish",
                { { "idle", 30 }, { "wander", 60 }, { "melee_attack", 100 }, { "patrol", 60 } },
                { "flee" } },

                // Coordinates with same-faction allies sharing its target: holds formation slot, then fights.
                { "squad_martial",
                    { { "idle", 5 }, { "wander", 15 }, { "hunt", 40 }, { "follow_squad_order", 100 }, { "melee_attack", 100 }, { "patrol", 60 } },
                    { "flee" } }
};

const BrainDef* FindBrain(const std::string& id)
{
    for (size_t i = 0; i < G_BRAINS.size(); i++)
    {
        if (G_BRAINS[i].id == id)
        {
            return &G_BRAINS[i];
        }
    }
    return nullptr;
}

// Stand-ins until Guard, Patrol, and KeepDistance exist. Territorial and caster creatures act like
// predators for now. Delete this and the AIBrain enum once every archetype row sets brainId.
static std::string GetLegacyBrainId(AIBrain brain)
{
    if (brain == BRAIN_SKITTISH)
    {
        return "skittish";
    }
    if (brain == BRAIN_PREDATOR)
    {
        return "predator";
    }
    if (brain == BRAIN_TERRITORIAL)
    {
        return "predator";
    }
    if (brain == BRAIN_CASTER)
    {
        return "predator";
    }
    return "mindless";
}

const BrainDef* GetBrainForEnemy(const Enemy& enemy)
{
    if (!enemy.brainOverrideId.empty())
    {
        const BrainDef* overrideBrain = FindBrain(enemy.brainOverrideId);
        if (overrideBrain != nullptr)
        {
            return overrideBrain;
        }
    }
    const EnemyArchetype* archetype = FindEnemyArchetype(enemy.archetypeId);
    if (archetype == nullptr)
    {
        return FindBrain("mindless");
    }

    if (!archetype->brainId.empty())
    {
        const BrainDef* brain = FindBrain(archetype->brainId);
        if (brain != nullptr)
        {
            return brain;
        }
    }

    return FindBrain(GetLegacyBrainId(archetype->brain));
}