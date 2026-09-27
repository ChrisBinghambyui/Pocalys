#pragma once
#include "EnemyData.h"
#include "FactionData.h"
#include "VariantData.h"
#include "Item.h"
#include "Enemy.h"
#include <string>
#include <vector>

int CalculateSpawnWeight(const SpawnRule& rule, int current_floor);

// Applies flat per-floor growth to a base attribute value. Used for str/end/agi/intel/wil/per/lck.
int CalculateScaledAttribute(int baseValue, int current_floor);

// Rolls this archetype's weapon count, type, and material tier into real Item instances, folding in
// any weapon types the rolled variant (if any) grants. Empty vector if there's nothing to wield.
std::vector<Item> GenerateEnemyLoadout(const EnemyArchetype& archetype, const CreatureVariant* variant);

// Single home for archetype lookup by id. Null if the id is unknown.
const EnemyArchetype* FindEnemyArchetype(const std::string& id);

// Display name for an enemy, looked up from its archetype.
std::string GetEnemyDisplayName(const Enemy& enemy);

// Short readout of an enemy's disposition toward the player for the LOOK action: "hostile",
// "friendly", "neutral", or "dead". Same living-vs-archetype faction check enemy AI already uses.
std::string GetEnemyIntentText(const Enemy& enemy);

// Armor rating for an enemy, looked up from its archetype. 0 if the archetype is unknown.
int GetEnemyArmor(const Enemy& enemy);


// Energy gained per player action for this enemy, looked up from its archetype. 100 if the archetype is unknown.
int GetEnemySpeed(const Enemy& enemy);

// Weight for this enemy, looked up from its archetype. 0 if the archetype is unknown.
float GetEnemyWeight(const Enemy& enemy);

// Detection radius for an enemy: archetype base plus its variant's modifier if it has one.
int GetEnemyDetectionRadius(const Enemy& enemy);

// Resolves faction stance (hostile/allied/neutral) of actor toward target, checking actor's
// factionIds in priority order. First faction with an opinion (shared membership or an explicit
// relation) wins. STANCE_NEUTRAL if neither archetype is found or nothing has an opinion.
FactionStance GetStance(const Enemy& actor, const Enemy& target);

// Weighted pick from G_SPAWN_RULES for a 1-based floor number. Null if nothing can spawn.
const EnemyArchetype* PickSpawnArchetype(int current_floor);

// Builds a living Enemy from an archetype: scaled attributes/HP/MP/FP, glyph, color, rolled loadout.
// Also rolls a chance at a variant tag (Archer, Warrior, Master, etc, see VariantData.h) which can
// shift stats, armor, weight, detection radius, and add weapon types to the loadout roll.
Enemy CreateEnemy(const EnemyArchetype& archetype, int floorNumber, int x, int y);
