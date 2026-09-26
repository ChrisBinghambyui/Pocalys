#include "FactionData.h"

std::vector<FactionData> G_FACTIONS = {
    { "rat",       {},                                                             true,  {} },
    { "living",    { { "undead", STANCE_HOSTILE } },                               false, {} },
    { "skeleton",  {},                                                             false, {} },
    { "undead",    { { "living", STANCE_HOSTILE } },                               false, {} },
    { "insect",    {},                                                             false, {} },
    { "beast",     { { "insect", STANCE_HOSTILE } },                               false, {} },
    { "warlock",   {},                                                             false, {} },
    { "horror",    { { "living", STANCE_HOSTILE }, { "undead", STANCE_HOSTILE } }, false, {} },
    { "construct", { { "living", STANCE_HOSTILE }, { "undead", STANCE_HOSTILE } }, false, {} },
    { "guardian",  { { "living", STANCE_HOSTILE } },                               false, {} },

    { "zombie",  {}, false, {} },
    { "ghost",   {}, false, {} },
    { "vampire", {}, false, {} },
    { "lich",    {}, false, {} },
    { "knight",  {}, false, {} },
    { "dragon",  {}, false, {} },
    { "mummy",   {}, false, {} },

    { "peasant", {}, false, {} },
    { "soldier", {}, false, {} },
    { "bowman",  {}, false, {} },
    { "griffin", {}, false, {} },
    { "cleric",  {}, false, {} },
    { "angel",   {}, false, {} },

    { "sprite",  {}, false, {} },
    { "gremlin", {}, false, {} },
    { "golem",   {}, false, {} },
    { "mage",    {}, false, {} },
    { "naga",    {}, false, {} },
    { "djinn",   {}, false, {} },
    { "titan",   {}, false, {} },

    { "goblin",  {}, false, {} },
    { "orc",     {}, false, {} },
    { "ogre",    {}, false, {} },
    { "roc",     {}, false, {} },
    { "cyclops", {}, false, {} },

    { "demon", { { "living", STANCE_HOSTILE }, { "undead", STANCE_HOSTILE } }, false, {} },
    { "succubus", {}, false, {} },
    { "devil", {}, false, {} },
    { "fiend", {}, false, {} },
    { "nightmare", {}, false, {} },

    { "unicorn", {}, false, {} },
    { "minotaur", {}, false, {} },
    { "raider", {}, false, {} },
    { "hydra", {}, false, {} }
};

const FactionData* FindFaction(const std::string& id)
{
    for (size_t i = 0; i < G_FACTIONS.size(); i++)
    {
        if (G_FACTIONS[i].id == id)
        {
            return &G_FACTIONS[i];
        }
    }
    return nullptr;
}

static bool FactionListContains(const std::vector<std::string>& list, const std::string& id)
{
    for (size_t i = 0; i < list.size(); i++)
    {
        if (list[i] == id)
        {
            return true;
        }
    }
    return false;
}

FactionStance GetFactionStance(const std::vector<std::string>& actorFactionIds, const std::vector<std::string>& targetFactionIds)
{
    for (size_t i = 0; i < actorFactionIds.size(); i++)
    {
        const std::string& actorFactionId = actorFactionIds[i];

        if (FactionListContains(targetFactionIds, actorFactionId))
        {
            return STANCE_ALLIED; // Shared membership, resolved at this faction's priority slot
        }

        const FactionData* faction = FindFaction(actorFactionId);
        if (faction == nullptr)
        {
            continue;
        }

        for (size_t j = 0; j < faction->relations.size(); j++)
        {
            if (FactionListContains(targetFactionIds, faction->relations[j].factionId))
            {
                return faction->relations[j].stance;
            }
        }
    }

    return STANCE_NEUTRAL;
}
