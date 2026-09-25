#pragma once
#include <string>
#include <vector>

enum FactionStance
{
    STANCE_HOSTILE,
    STANCE_ALLIED,
    STANCE_NEUTRAL
};

struct FactionRelation
{
    std::string factionId;
    FactionStance stance;
};

struct FactionData
{
    std::string id;
    std::vector<FactionRelation> relations; // Explicit stance toward other factions. Unlisted = no opinion, falls through.
    bool isScavenger = false;               // Can this faction show up near an unattended corpse?
    std::vector<std::string> scavengesFromIds; // Which factions' corpses draw it. Empty + isScavenger = scavenges anything.
};

extern std::vector<FactionData> G_FACTIONS;

// Null if no faction has this id.
const FactionData* FindFaction(const std::string& id);

// Resolves actor's stance toward target by walking actor's factionIds in priority order (index 0 first).
// The first faction that either shares membership with target (implicit ALLIED) or has an explicit
// relation naming one of target's factions wins. STANCE_NEUTRAL if nothing has an opinion.
FactionStance GetFactionStance(const std::vector<std::string>& actorFactionIds, const std::vector<std::string>& targetFactionIds);