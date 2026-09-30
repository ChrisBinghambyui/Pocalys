#pragma once
#include "raylib.h"
#include <vector>
#include "Item.h"
#include <string>

struct Enemy {
    float x; // Tile-space position, same convention as Player: the enemy's center is (x + 0.5, y + 0.5)
    float y;
    int hp;
    int maxHp;
    int stamina;
    int maxStamina;
    int mana;
    int maxMana;
    int str;
    int end;
    int agi;
    int intel;
    int wil;
    int per;
    int lck;
    char symbol; // What character represents this enemy (e.g., 'g' for goblin)
    Color color; // What color to draw them
    bool isDead; // Corpses stay on the map but stop blocking movement/attacks
    std::string archetypeId; // Key into G_ENEMY_ARCHETYPES. Name, armor, and damage are looked up from it.
    int spawnFloor;          // 1-based floor this enemy spawned on. Pair with the archetype to scale damage.
    std::vector<Item> inventory; // Loot available once this corpse is opened as a container
    int energy = 0; // Accumulates via AdvanceEnemyEnergy (EnemyTurns.h). Crosses ACTION_THRESHOLD to act.
    float homeX = 0.0f; // Spawn position, leash anchor for guard-style behaviors later.
    float homeY = 0.0f;
    std::string variantId = ""; // Empty if no variant tag rolled. See VariantData.h.

    // Set by PopulateFloor (FloorPopulation.cpp) for enemies spawned as part of a placed group.
    int groupId = -1;                 // Members of one group share an id and squad together whatever their archetype. -1 = ungrouped.
    std::string brainOverrideId = ""; // Key into G_BRAINS. Beats the archetype's brain when not empty.
    int detectionBonus = 0;           // Floor modifier bonus, baked in at spawn so a revisited floor stays consistent

    int patrolRoomIndex = -1; // Index into the floor's room list. -1 = no patrol goal, needs a fresh pick.

    // Squad formation state, set by UpdateSquads (Squad.cpp), read by follow_squad_order.
    bool hasSquadSlot = false;
    float squadSlotX = 0.0f;
    float squadSlotY = 0.0f;

    // Navmesh path, set by RequestPath (BehaviorData.cpp) and consumed by FollowStoredPath.
    std::vector<Vector2> path;
    int pathIndex = 0;
    float repathTimer = 0.0f;
    float pathTargetX = 0.0f; // Where the current path was aimed, so Hunt can tell when the target has drifted far enough to justify a repath
    float pathTargetY = 0.0f;

    // Runtime AI state, driven by EnemyBehavior.cpp. Nothing here is archetype data.
    std::string behaviorId = "";  // Key into G_BEHAVIORS. Empty = needs a new pick next frame.
    float behaviorTimer = 0.0f;   // Seconds the current behavior has run
    float behaviorLimit = 0.0f;   // Seconds before it gives up. 0 = no limit.
    int targetIndex = -1;         // Index into the floor's enemy vector. -1 = no enemy target.
    bool targetIsPlayer = false;
    bool hadThreat = false;       // Last frame's target state, used to detect "target appeared or vanished"
    int lastHp = -1;              // Last frame's HP, used to detect "took damage"
    float attackCooldown = 0.0f;
    float moveDirX = 0.0f;        // Set by behaviors, consumed by the shared movement step
    float moveDirY = 0.0f;
    float moveSpeedScale = 1.0f;
    float goalX = 0.0f;           // Wander destination
    float goalY = 0.0f;
    bool hasGoal = false;
};
