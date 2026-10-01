#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include <raylib.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include "DungeonData.h"
#include "Player.h"
#include "Enemy.h"
#include "Combat.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include "ItemData.h"
#include "CharacterGenerator.h"
#include "TitleScreen.h"
#include "EnemyFactory.h"
#include "FloorPopulation.h"
#include "DungeonGen.h"
#include "StashData.h"
#include "StashUI.h"
#include "FloorModifierData.h"
#include "MetaSave.h"
#include "RosterData.h"
#include "RosterUI.h"
#include "FeatData.h"
#include "AbilityHotbar.h"
#include "AbilityData.h"
#include "HotbarUI.h"
#include "MenuHubUI.h"
#include "TargetingUI.h"
#include "WeaponVisual.h"
#include "UIScale.h"
#include "ArrowData.h"
#include "EnemyBehavior.h"
#include "FactionConflict.h"
#include "LineOfSight.h"
#include "NavMesh.h"
#include "Squad.h"
#include "Alert.h"
#include <string>

const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 70;
const int MAX_ROOMS = 15;
const int PLAYER_VISION_RADIUS = 8; // Tiles of line-of-sight range for fog-of-war reveal
const float PLAYER_MOVE_SPEED = 5.0f;       // Tiles per second, placeholder until playtested
const float PLAYER_COLLISION_RADIUS = 0.3f; // Tile units, checked against wall tiles only for now
const float PICKUP_RANGE = 0.8f;            // Tile units, distance for G to reach a ground item
const float MELEE_ATTACK_RANGE = 1.2f;      // Tile units, distance for a left-click attack to reach an enemy
const float MELEE_ATTACK_FACING_COS = 0.5f; // Minimum dot product between facing and to-enemy direction (~120 degree cone)
const float INTERACT_RANGE = 1.6f;          // Tile units, reach for SPEAK / GRAB / STEAL. LOOK ignores it. 1.6 covers a diagonal neighbor.
const float ENEMY_CLICK_RADIUS = 0.6f;      // Tile units, forgiving radius around an enemy's true position for mouse-click interaction

enum GameState {
    STATE_TITLE,
    STATE_TAVERN,
    STATE_GAMEPLAY,
    STATE_STASH,
    STATE_ROSTER
};


// --- FLOOR WIPE TRANSITION ---
void PerformFloorTransition(bool isDescending) {
    const float transitionDuration = 0.25f;
    float timer = 0.0f;
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    while (timer < transitionDuration && !WindowShouldClose()) {
        float dt = GetFrameTime();
        timer += dt;
        float progress = timer / transitionDuration;
        if (progress > 1.0f) progress = 1.0f;

        BeginDrawing();

        if (isDescending) {
            int wipeHeight = (int)(screenH * progress);
            DrawRectangle(0, 0, screenW, wipeHeight, BLACK);
            DrawRectangle(0, wipeHeight - 4, screenW, 4, GOLD);
        }
        else {
            int wipeHeight = (int)(screenH * progress);
            int startY = screenH - wipeHeight;
            DrawRectangle(0, startY, screenW, wipeHeight, BLACK);
            DrawRectangle(0, startY, screenW, 4, GOLD);
        }

        EndDrawing();
    }
}


std::string GetEnemyName(const Enemy& enemy) {
    return GetEnemyDisplayName(enemy);
}

std::string GetGroundItemName(const Item& item) {
    if (item.weaponTypeId >= 0) {
        std::string mat = (item.materialTier >= 0 && item.materialTier < (int)G_MATERIAL_TIERS.size())
            ? G_MATERIAL_TIERS[item.materialTier].name + " "
            : "";
        return mat + G_WEAPON_TYPES[item.weaponTypeId].name;
    }
    else if (item.ammoTypeId >= 0) {
        const ArrowType* arrow = FindArrowType(item.ammoTypeId);
        if (arrow != nullptr) {
            return arrow->name;
        }
        return "arrows";
    }
    else if (!item.archetypeId.empty()) {
        for (const auto& arch : G_ITEM_ARCHETYPES) {
            if (arch.id == item.archetypeId) return arch.name;
        }
        return item.archetypeId;
    }
    return "item";
}

void GenerateFloor(TileType map[MAP_WIDTH][MAP_HEIGHT], bool explored[MAP_WIDTH][MAP_HEIGHT], std::vector<Room>& rooms, Player& player, std::vector<Enemy>& enemies, const FloorParams& params) {
    rooms.clear();
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            map[x][y] = TILE_WALL;
            explored[x][y] = false;
        }
    }

    std::vector<TileType> layout;
    BuildDungeonLayout(MAP_WIDTH, MAP_HEIGHT, MAX_ROOMS, layout, rooms);
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            map[x][y] = layout[x * MAP_HEIGHT + y];
        }
    }

    map[rooms[0].centerX()][rooms[0].centerY()] = TILE_STAIR_UP;

    int furthestIdx = 0; float maxDist = 0;
    for (size_t i = 1; i < rooms.size(); i++) {
        float dist = std::pow(rooms[i].centerX() - rooms[0].centerX(), 2) + std::pow(rooms[i].centerY() - rooms[0].centerY(), 2);
        if (dist > maxDist) { maxDist = dist; furthestIdx = i; }
    }
    map[rooms[furthestIdx].centerX()][rooms[furthestIdx].centerY()] = TILE_STAIR_DOWN;

    // Every tenth floor gets an extraction point one tile beside the down stairs
    if (params.floorInChunk == FLOORS_PER_CHUNK)
    {
        map[rooms[furthestIdx].centerX() - 1][rooms[furthestIdx].centerY()] = TILE_EXTRACT;
    }

    player.x = rooms[0].centerX() + 1;
    player.y = rooms[0].centerY();

    PopulateFloor(params, rooms, furthestIdx, enemies);
}

void ApplyProfileToPlayer(const CharacterProfile& profile, Player& player) {
    player = Player(); // Wipes level, progress, feats, boons, skills, amulets, rings from any previous run
    player.name = profile.name;
    player.className = profile.className;
    player.birthsign = profile.birthsign;
    player.str = profile.str; player.end = profile.end; player.agi = profile.agi;
    player.intel = profile.intel; player.wil = profile.wil; player.per = profile.per; player.lck = profile.lck;
    player.maxHp = profile.maxHp; player.hp = profile.hp;
    player.maxStamina = profile.maxStamina; player.stamina = profile.stamina;
    player.maxMana = profile.maxMana; player.mana = profile.mana;
    player.inventory = profile.inventory;
    for (int i = 0; i < SLOT_SINGLE_COUNT; i++) player.equippedSlots[i] = profile.equippedSlots[i];
    player.equippedAmulets.clear(); player.equippedRings.clear();

    player.initSkills();
    player.majorSkills = profile.majorSkills;
    player.minorSkills = profile.minorSkills;
    player.applySkillTiers();

    // Racial bonuses apply after class tiers, so a SET bonus overrides whatever the class gave
    player.raceId = profile.raceId;
    for (size_t i = 0; i < profile.raceSkillBonuses.size(); i++)
    {
        const SkillBonus& bonus = profile.raceSkillBonuses[i];
        if (bonus.skillId >= 0 && bonus.skillId < (int)player.skills.size())
        {
            if (bonus.mode == SKILL_BONUS_SET)
            {
                player.skills[bonus.skillId].level = bonus.amount;
            }
            else
            {
                player.skills[bonus.skillId].level += bonus.amount;
            }
        }
    }

    // Innate racial ability, granted as a feat
    const RaceData* race = FindRace(profile.raceId);
    if (race != nullptr)
    {
        player.activeFeats.push_back(race->racialFeatId);
    }
}

// Circle-vs-tile check for player wall collision. Cheap approximation: test the four cardinal
// points of the circle instead of every tile it could overlap. Good enough for a modest collision
// radius against grid-aligned walls.
static bool IsWorldPositionWalkable(TileType map[MAP_WIDTH][MAP_HEIGHT], float centerX, float centerY, float radius)
{
    float checkPoints[4][2] = {
        { centerX - radius, centerY },
        { centerX + radius, centerY },
        { centerX, centerY - radius },
        { centerX, centerY + radius }
    };

    for (int i = 0; i < 4; i++)
    {
        int tileX = (int)checkPoints[i][0];
        int tileY = (int)checkPoints[i][1];
        if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT)
        {
            return false;
        }
        TileType tile = map[tileX][tileY];
        if (tile != TILE_FLOOR && tile != TILE_STAIR_UP && tile != TILE_STAIR_DOWN && tile != TILE_EXTRACT)
        {
            return false;
        }
    }

    return true;
}

// True if the tile under the player's collision center is this type. Bounds-checked, because noclip can walk off the map.
static bool IsPlayerOnTile(TileType map[MAP_WIDTH][MAP_HEIGHT], const Player& player, TileType tile)
{
    int tileX = (int)(player.x + 0.5f);
    int tileY = (int)(player.y + 0.5f);
    if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT)
    {
        return false;
    }
    return map[tileX][tileY] == tile;
}

int main()
{
    const int tileSize = 24;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    int currentMonitor = GetCurrentMonitor();
    int screenWidth = GetMonitorWidth(currentMonitor);
    int screenHeight = GetMonitorHeight(currentMonitor);

    InitWindow(screenWidth, screenHeight, "Scrolls & Steel");
    SetExitKey(KEY_NULL);
    ToggleBorderlessWindowed();
    SetTargetFPS(60);
    InitTitleScreen();
    ResolveWeaponSkillIds();
    ResolveEnemyArchetypeDefaults();
    LoadMetaGame();

    TileType map[MAP_WIDTH][MAP_HEIGHT];
    bool explored[MAP_WIDTH][MAP_HEIGHT] = { false };
    std::vector<Room> rooms;
    std::vector<LevelState> dungeon;
    int currentFloor = 0;
    int floorAlert = 0;
    std::vector<Enemy> enemies;
    std::vector<GroundItem> groundItems;
    std::string actionMessage = "";
    const ExpeditionDef* currentExpedition = nullptr;
    StashLoadoutState stashLoadout;
    bool stashPackDone = false;  // True for the one frame after the pack screen, forces the tavern confirm through
    int stashPatronIndex = 0;    // Patron picked before the pack screen, restored after it (mouse hover can move the selection)
    bool extractArmed = false;   // First Space on the extraction tile arms it, the second one leaves
    bool showExtractScreen = false;
    RosterViewState rosterView;
    int pendingRosterIndex = -1;   // Veteran picked on the roster screen, -1 = a fresh recruit is starting instead
    bool rosterPickReady = false;  // True for the one frame after the roster screen, forces the tavern confirm through
    std::string extractSummary = "";
    std::vector<std::string> activeModifierIds; // Modifier ids (G_FLOOR_MODIFIERS) in force this expedition. Contracts fill this later.
    int playerVisionMod = 0;                    // Summed visionRadiusMod of the active modifiers

    NavMesh navMesh;
    bool showNavDebug = false;
    auto rebuildNavMesh = [&]() {
        navMesh.Build(MAP_WIDTH, MAP_HEIGHT, [&](int tileX, int tileY) -> bool {
            TileType tile = map[tileX][tileY];
            return tile == TILE_FLOOR || tile == TILE_STAIR_UP || tile == TILE_STAIR_DOWN || tile == TILE_EXTRACT;
            });
        };

    enum ActionMode { MODE_GRAB, MODE_LOOK, MODE_SPEAK, MODE_STEAL };
    ActionMode currentMode = MODE_LOOK;
    const char* modeNames[] = { "GRAB", "LOOK", "SPEAK", "STEAL" };

    Player player;
    GameState currentState = STATE_TITLE;

    std::vector<CharacterProfile> tavernCandidates;
    int selectedCandidate = 0;

    bool enableFog = true;
    bool showInventory = false;
    int selectedItemIndex = 0;
    bool showPauseMenu = false;
    int pauseMenuSelection = 0;

    bool showMenuHub = false;
    MenuTab currentMenuTab = MENU_TAB_INVENTORY;
    bool showDeathScreen = false;
    std::string aimingAbilityId = "";
    WeaponSwingState weaponSwing;

    Camera2D camera = { 0 };
    camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.5f;

    bool keepRunning = true;
    while (keepRunning && !WindowShouldClose())
    {

        // --- TITLE STATE ---
        if (currentState == STATE_TITLE)
        {
            TitleAction titleAction = UpdateAndDrawTitleScreen();
            if (titleAction == TITLE_ACTION_NEW_GAME)
            {
                tavernCandidates = GenerateTavernCandidates(6);
                selectedCandidate = 0;
                currentState = STATE_TAVERN;
            }
            else if (titleAction == TITLE_ACTION_OPTIONS)
            {
                actionMessage = "Options coming soon!";
            }
            else if (titleAction == TITLE_ACTION_QUIT)
            {
                keepRunning = false;
            }
            continue;
        }

        // --- TAVERN / CHARACTER SELECTION STATE ---
        if (currentState == STATE_TAVERN)
        {
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                if (selectedCandidate > 0) selectedCandidate--;
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                if (selectedCandidate < (int)tavernCandidates.size() - 1) selectedCandidate++;
            }
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                if (selectedCandidate >= 3) selectedCandidate -= 3;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                if (selectedCandidate + 3 < (int)tavernCandidates.size()) selectedCandidate += 3;
            }
            if (IsKeyPressed(KEY_R)) {
                tavernCandidates = GenerateTavernCandidates(6);
            }

            if (IsKeyPressed(KEY_V) && !G_ROSTER.empty())
            {
                rosterView = RosterViewState();
                currentState = STATE_ROSTER;
            }

            for (int k = 0; k < 6; k++) {
                if (IsKeyPressed(KEY_ONE + k)) selectedCandidate = k;
            }

            bool confirmSelection = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);

            BeginDrawing();
            ClearBackground(Color{ 18, 12, 14, 255 });

            float uiScale = GetUIScale();
            int titleFontSize = (int)(32 * uiScale);
            DrawText("THE RUSTY ANVIL TAVERN", GetScreenWidth() / 2 - MeasureText("THE RUSTY ANVIL TAVERN", titleFontSize) / 2, (int)(30 * uiScale), titleFontSize, GOLD);
            int subFontSize = (int)(22 * uiScale);
            const char* subtitleText = "Select a patron to descend into the dungeon...";
            DrawText(subtitleText, GetScreenWidth() / 2 - MeasureText(subtitleText, subFontSize) / 2, (int)(80 * uiScale), subFontSize, LIGHTGRAY);

            Vector2 mousePos = GetMousePosition();
            int marginX = (int)(30 * uiScale);
            int marginY = (int)(25 * uiScale);
            int gridEdge = (int)(60 * uiScale);
            int cardW = (GetScreenWidth() - gridEdge * 2 - marginX * 2) / 3;
            int cardH = (int)(cardW * 0.62f);
            int startX = gridEdge;
            int startY = (int)(140 * uiScale);
            int nameFontSize = (int)(24 * uiScale);
            int lineFontSize = (int)(19 * uiScale);
            int smallFontSize = (int)(18 * uiScale);
            int cardPadding = (int)(16 * uiScale);

            for (int i = 0; i < (int)tavernCandidates.size(); i++) {
                const auto& cand = tavernCandidates[i];
                int col = i % 3; int row = i / 3;
                int cx = startX + col * (cardW + marginX);
                int cy = startY + row * (cardH + marginY);

                Rectangle cardRect = { (float)cx, (float)cy, (float)cardW, (float)cardH };
                bool isHovered = CheckCollisionPointRec(mousePos, cardRect);

                if (isHovered) {
                    selectedCandidate = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) confirmSelection = true;
                }

                bool isSelected = (i == selectedCandidate);
                Color bgCol = isSelected ? Color{ 45, 32, 28, 255 } : Color{ 28, 22, 24, 255 };
                Color borderCol = isSelected ? GOLD : GRAY;

                DrawRectangle(cx, cy, cardW, cardH, bgCol);
                DrawRectangleLinesEx(cardRect, isSelected ? 4.0f : 1.5f, borderCol);

                int lineY = cy + cardPadding;
                DrawText(TextFormat("%d. %s", i + 1, cand.name.c_str()), cx + cardPadding, lineY, nameFontSize, isSelected ? YELLOW : WHITE);
                lineY += (int)(nameFontSize * 1.4f);
                DrawText(cand.className.c_str(), cx + cardPadding, lineY, lineFontSize, ORANGE);
                lineY += (int)(lineFontSize * 1.3f);
                DrawText(TextFormat("Birthsign: %s", cand.birthsign.c_str()), cx + cardPadding, lineY, smallFontSize, ORANGE);
                lineY += (int)(lineFontSize * 1.4f);
                DrawText(TextFormat("HP: %d  SP: %d  MP: %d", cand.maxHp, cand.maxStamina, cand.maxMana), cx + cardPadding, lineY, lineFontSize, GREEN);
                lineY += (int)(lineFontSize * 1.4f);
                DrawText(TextFormat("STR:%d END:%d AGI:%d INT:%d", cand.str, cand.end, cand.agi, cand.intel), cx + cardPadding, lineY, smallFontSize, LIGHTGRAY);
                lineY += (int)(smallFontSize * 1.3f);
                DrawText(TextFormat("WIL:%d PER:%d LCK:%d", cand.wil, cand.per, cand.lck), cx + cardPadding, lineY, smallFontSize, LIGHTGRAY);
                lineY += (int)(smallFontSize * 1.5f);

                const RaceData* candRace = FindRace(cand.raceId);
                if (candRace != nullptr)
                {
                    DrawText(TextFormat("Race: %s", candRace->name.c_str()), cx + cardPadding, lineY, lineFontSize, VIOLET);
                }
            }

            // Detail panel for the highlighted patron (mouse hover or keyboard selection)
            if (selectedCandidate >= 0 && selectedCandidate < (int)tavernCandidates.size())
            {
                const CharacterProfile& sel = tavernCandidates[selectedCandidate];
                const RaceData* selRace = FindRace(sel.raceId);
                int panelX = startX;
                int panelY = startY + 2 * (cardH + marginY) + (int)(10 * uiScale);
                int panelW = cardW * 3 + marginX * 2;
                int panelH = GetScreenHeight() - panelY - (int)(90 * uiScale);
                if (panelH < (int)(160 * uiScale)) {
                    panelH = (int)(160 * uiScale);
                }

                DrawRectangle(panelX, panelY, panelW, panelH, Color{ 28, 22, 24, 255 });
                DrawRectangleLinesEx(Rectangle{ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 1.5f, GOLD);

                if (selRace != nullptr)
                {
                    int detailPadding = (int)(18 * uiScale);
                    int detailY = panelY + detailPadding;
                    DrawText(("RACE: " + selRace->name).c_str(), panelX + detailPadding, detailY, nameFontSize, VIOLET);
                    detailY += (int)(nameFontSize * 1.5f);
                    DrawText(selRace->description.c_str(), panelX + detailPadding, detailY, lineFontSize, LIGHTGRAY);
                    detailY += (int)(lineFontSize * 1.8f);

                    const Feat* racialFeat = FindFeat(selRace->racialFeatId);
                    if (racialFeat != nullptr)
                    {
                        DrawText(racialFeat->name.c_str(), panelX + detailPadding, detailY, lineFontSize, ORANGE);
                        detailY += (int)(lineFontSize * 1.5f);
                        DrawText(racialFeat->description.c_str(), panelX + detailPadding, detailY, smallFontSize, LIGHTGRAY);
                        detailY += (int)(smallFontSize * 1.7f);
                    }

                    std::string bonusText = "SKILL BONUSES:  " + DescribeSkillBonuses(sel.raceSkillBonuses);
                    DrawText(bonusText.c_str(), panelX + detailPadding, detailY, smallFontSize, SKYBLUE);
                }
            }

            std::string hint = "[WASD / Arrows / 1-6] Select  |  [ENTER / Click] Begin Quest  |  [R] Reroll Patrons";
            if (!G_ROSTER.empty())
            {
                hint += "  |  [V] Veterans (" + std::to_string((int)G_ROSTER.size()) + ")";
            }
            int hintFontSize = (int)(22 * uiScale);
            DrawText(hint.c_str(), GetScreenWidth() / 2 - MeasureText(hint.c_str(), hintFontSize) / 2, GetScreenHeight() - (int)(50 * uiScale), hintFontSize, GOLD);
            EndDrawing();

            if (rosterPickReady)
            {
                rosterPickReady = false;
                confirmSelection = true;
            }

            if (confirmSelection && !stashPackDone && !G_STASH.items.empty())
            {
                stashPatronIndex = selectedCandidate;
                stashLoadout = StashLoadoutState();
                currentState = STATE_STASH;
                confirmSelection = false;
            }
            if (stashPackDone)
            {
                selectedCandidate = stashPatronIndex;
                confirmSelection = true;
            }

            if (confirmSelection) {
                if (pendingRosterIndex >= 0)
                {
                    TakeFromRoster(pendingRosterIndex, player);
                    pendingRosterIndex = -1;
                    SaveMetaGame(); // The veteran is now out on an expedition and off the roster until they extract
                }
                else
                {
                    ApplyProfileToPlayer(tavernCandidates[selectedCandidate], player);
                }
                FillEmptyHotbarSlots(player);
                if (stashPackDone)
                {
                    WithdrawFlaggedFromStash(stashLoadout.packed, player);
                    SaveMetaGame();
                    stashPackDone = false;
                }

                dungeon.clear();
                currentFloor = 0;
                groundItems.clear();
                currentExpedition = PickRandomExpedition();
                activeModifierIds.clear();
                if (!DEBUG_FORCED_MODIFIER_ID.empty())
                {
                    activeModifierIds.push_back(DEBUG_FORCED_MODIFIER_ID);
                }
                playerVisionMod = BuildFloorParams(currentExpedition, activeModifierIds, currentFloor + 1).visionRadiusMod;
                GenerateFloor(map, explored, rooms, player, enemies, BuildFloorParams(currentExpedition, activeModifierIds, currentFloor + 1));
                rebuildNavMesh();

                LevelState firstFloor;
                for (int x = 0; x < MAP_WIDTH; x++) {
                    for (int y = 0; y < MAP_HEIGHT; y++) {
                        firstFloor.savedMap[x][y] = map[x][y];
                        firstFloor.savedExplored[x][y] = explored[x][y];
                    }
                }
                firstFloor.savedEnemies = enemies;
                firstFloor.savedItems = groundItems;
                firstFloor.alertLevel = floorAlert;
                dungeon.push_back(firstFloor);

                camera.target = { (float)player.x * tileSize + (tileSize / 2.0f), (float)player.y * tileSize + (tileSize / 2.0f) };
                actionMessage = "You descend into the catacombs as " + player.name + " the " + player.className + ".";
                if (currentExpedition != nullptr)
                {
                    actionMessage = "Expedition: " + currentExpedition->name + ". " + actionMessage;
                }
                currentState = STATE_GAMEPLAY;
            }
            continue;
        }

        // --- ROSTER STATE ---
        if (currentState == STATE_ROSTER)
        {
            int rosterPick = -1;
            RosterUIAction rosterAction = UpdateAndDrawRoster(rosterView, rosterPick);
            if (rosterAction == ROSTER_UI_BEGIN)
            {
                pendingRosterIndex = rosterPick;
                rosterPickReady = true;
                currentState = STATE_TAVERN;
            }
            else if (rosterAction == ROSTER_UI_BACK)
            {
                currentState = STATE_TAVERN;
            }
            continue;
        }

        // --- STASH LOADOUT STATE ---
        if (currentState == STATE_STASH)
        {
            StashUIAction stashAction = UpdateAndDrawStashLoadout(stashLoadout);
            if (stashAction == STASH_UI_BEGIN)
            {
                stashPackDone = true;
                currentState = STATE_TAVERN;
            }
            else if (stashAction == STASH_UI_BACK)
            {
                pendingRosterIndex = -1; // Backing out of the pack screen cancels a veteran pick too
                currentState = STATE_TAVERN;
            }
            continue;
        }

        // --- EXTRACTION SUMMARY ---
        if (showExtractScreen)
        {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                showExtractScreen = false;
                tavernCandidates = GenerateTavernCandidates(6);
                selectedCandidate = 0;
                currentState = STATE_TAVERN;
            }

            BeginDrawing();
            ClearBackground(BLACK);
            float extractScale = GetUIScale();
            const char* extractTitle = "EXTRACTED";
            int extractTitleFont = (int)(90 * extractScale);
            DrawText(extractTitle, GetScreenWidth() / 2 - MeasureText(extractTitle, extractTitleFont) / 2, GetScreenHeight() / 2 - extractTitleFont, extractTitleFont, GOLD);
            int extractLineFont = (int)(28 * extractScale);
            DrawText(extractSummary.c_str(), GetScreenWidth() / 2 - MeasureText(extractSummary.c_str(), extractLineFont) / 2, GetScreenHeight() / 2 + (int)(10 * extractScale), extractLineFont, LIGHTGRAY);
            const char* extractHint = "Press Enter to return to the tavern";
            int extractHintFont = (int)(24 * extractScale);
            DrawText(extractHint, GetScreenWidth() / 2 - MeasureText(extractHint, extractHintFont) / 2, GetScreenHeight() / 2 + (int)(70 * extractScale), extractHintFont, GRAY);
            EndDrawing();
            continue;
        }

        if (showDeathScreen)
        {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
            {
                showDeathScreen = false;
                ResetTitleScreen();
                currentState = STATE_TITLE;
            }

            BeginDrawing();
            ClearBackground(BLACK);
            float deathScale = GetUIScale();
            const char* deathText = "YOU DIED";
            int deathFontSize = (int)(100 * deathScale);
            int deathWidth = MeasureText(deathText, deathFontSize);
            DrawText(deathText, GetScreenWidth() / 2 - deathWidth / 2, GetScreenHeight() / 2 - deathFontSize / 2, deathFontSize, MAROON);

            const char* deathHint = "Press Enter to return to the title";
            int hintFontSize = (int)(28 * deathScale);
            int hintWidth = MeasureText(deathHint, hintFontSize);
            DrawText(deathHint, GetScreenWidth() / 2 - hintWidth / 2, GetScreenHeight() / 2 + deathFontSize / 2 + (int)(20 * deathScale), hintFontSize, GRAY);
            EndDrawing();
            continue;
        }

        // --- GAMEPLAY INPUT & UPDATE ---
        camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };


        if (player.attackCooldown > 0.0f)
        {
            player.attackCooldown -= GetFrameTime();
        }

        UpdateWeaponSwing(weaponSwing, GetFrameTime());

        Vector2 mouseWorldForFacing = GetScreenToWorld2D(GetMousePosition(), camera);
        float facingDeltaX = mouseWorldForFacing.x - (player.x * tileSize + tileSize / 2.0f);
        float facingDeltaY = mouseWorldForFacing.y - (player.y * tileSize + tileSize / 2.0f);
        float facingLength = sqrtf(facingDeltaX * facingDeltaX + facingDeltaY * facingDeltaY);
        if (facingLength > 0.001f) {
            player.facingX = facingDeltaX / facingLength;
            player.facingY = facingDeltaY / facingLength;
        }

        auto isOpaque = [&](int checkX, int checkY) -> bool {
            if (checkX < 0 || checkX >= MAP_WIDTH || checkY < 0 || checkY >= MAP_HEIGHT) {
                return true; // Out of bounds blocks sight the same as a wall
            }
            return map[checkX][checkY] == TILE_WALL;
            };

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (!aimingAbilityId.empty()) {
                aimingAbilityId = "";
            }
            else {
                showPauseMenu = !showPauseMenu;
                if (showPauseMenu) {
                    showInventory = false;
                    showMenuHub = false;
                }
            }
        }

        if (IsKeyPressed(KEY_TAB) && !showPauseMenu) {
            showMenuHub = !showMenuHub;
            aimingAbilityId = "";
        }

        if (showMenuHub) {
            if (IsKeyPressed(KEY_LEFT_BRACKET) || IsKeyPressed(KEY_LEFT)) {
                currentMenuTab = (MenuTab)((currentMenuTab + MENU_TAB_COUNT - 1) % MENU_TAB_COUNT);
            }
            if (IsKeyPressed(KEY_RIGHT_BRACKET) || IsKeyPressed(KEY_RIGHT)) {
                currentMenuTab = (MenuTab)((currentMenuTab + 1) % MENU_TAB_COUNT);
            }
        }

        showInventory = showMenuHub && currentMenuTab == MENU_TAB_INVENTORY;
        bool menuOpen = showPauseMenu || showMenuHub;

        if (!showMenuHub && !showPauseMenu) {
            for (int slot = 0; slot < (int)player.hotbar.size(); slot++) {
                char keyChar = GetHotbarKeyLabel(slot);
                int slotKey = KEY_ZERO + (keyChar - '0');
                if (!IsKeyPressed(slotKey)) {
                    continue;
                }

                std::string abilityId = player.hotbar[slot];
                if (abilityId.empty() || !IsAbilityAvailable(player, abilityId)) {
                    continue;
                }

                if (aimingAbilityId == abilityId) {
                    aimingAbilityId = "";
                    continue;
                }

                const AbilityDef* pressedAbility = FindAbility(abilityId);
                if (pressedAbility != nullptr && pressedAbility->shape == ABILITY_SHAPE_SELF) {
                    actionMessage = "Used " + pressedAbility->name + ". (Ability execution isn't wired in yet.)";
                    aimingAbilityId = "";
                }
                else {
                    aimingAbilityId = abilityId;
                }
            }
        }

        if (showPauseMenu) {
            const int pauseOptionCount = 6;
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                pauseMenuSelection--;
                if (pauseMenuSelection < 0) pauseMenuSelection = pauseOptionCount - 1;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                pauseMenuSelection++;
                if (pauseMenuSelection >= pauseOptionCount) pauseMenuSelection = 0;
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E)) {
                if (pauseMenuSelection == 0) showPauseMenu = false;
                else if (pauseMenuSelection == 1) { actionMessage = "Save feature not implemented yet."; showPauseMenu = false; }
                else if (pauseMenuSelection == 2) { actionMessage = "Load feature not implemented yet."; showPauseMenu = false; }
                else if (pauseMenuSelection == 3) { actionMessage = "Options feature not implemented yet."; showPauseMenu = false; }
                else if (pauseMenuSelection == 4) { currentState = STATE_TITLE; showPauseMenu = false; }
                else if (pauseMenuSelection == 5) keepRunning = false;
            }
        }
        else if (!menuOpen) {
            float moveInputX = 0.0f;
            float moveInputY = 0.0f;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) moveInputX += 1.0f;
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) moveInputX -= 1.0f;
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) moveInputY += 1.0f;
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) moveInputY -= 1.0f;

            if (moveInputX != 0.0f || moveInputY != 0.0f) {
                float inputLength = sqrtf(moveInputX * moveInputX + moveInputY * moveInputY);
                moveInputX /= inputLength;
                moveInputY /= inputLength;

                float moveDistance = PLAYER_MOVE_SPEED * GetFrameTime();

                float candidateX = player.x + moveInputX * moveDistance;
                if (!enableFog || IsWorldPositionWalkable(map, candidateX + 0.5f, player.y + 0.5f, PLAYER_COLLISION_RADIUS)) {
                    player.x = candidateX;
                }

                float candidateY = player.y + moveInputY * moveDistance;
                if (!enableFog || IsWorldPositionWalkable(map, player.x + 0.5f, candidateY + 0.5f, PLAYER_COLLISION_RADIUS)) {
                    player.y = candidateY;
                }
            }

            if (IsKeyPressed(KEY_G)) {
                for (auto it = groundItems.begin(); it != groundItems.end(); ++it) {
                    float itemCenterX = (float)it->x + 0.5f;
                    float itemCenterY = (float)it->y + 0.5f;
                    float dx = itemCenterX - (player.x + 0.5f);
                    float dy = itemCenterY - (player.y + 0.5f);
                    if (dx * dx + dy * dy <= PICKUP_RANGE * PICKUP_RANGE) {
                        player.inventory.push_back(it->item);
                        actionMessage = "Picked up item.";
                        groundItems.erase(it);
                        break;
                    }
                }
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !aimingAbilityId.empty())
            {
                const AbilityDef* firedAbility = FindAbility(aimingAbilityId);
                std::string firedName = "ability";
                if (firedAbility != nullptr)
                {
                    firedName = firedAbility->name;
                }
                actionMessage = "Fired " + firedName + ". (Ability execution isn't wired in yet.)";
                aimingAbilityId = "";
            }
            else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && player.attackCooldown <= 0.0f)
            {
                float playerCenterX = player.x + 0.5f;
                float playerCenterY = player.y + 0.5f;

                Enemy* targetEnemy = nullptr;
                float closestDistSq = MELEE_ATTACK_RANGE * MELEE_ATTACK_RANGE;

                for (size_t i = 0; i < enemies.size(); i++) {
                    if (enemies[i].isDead) {
                        continue;
                    }

                    float toEnemyX = (enemies[i].x + 0.5f) - playerCenterX;
                    float toEnemyY = (enemies[i].y + 0.5f) - playerCenterY;
                    float distSq = toEnemyX * toEnemyX + toEnemyY * toEnemyY;
                    if (distSq > closestDistSq) {
                        continue;
                    }

                    closestDistSq = distSq;
                    targetEnemy = &enemies[i];
                }

                float swingSeconds = GetAttackCooldownSeconds(player);
                player.attackCooldown = swingSeconds; // A whiff costs the same time as a hit
                DamageType swingType = ChooseSwingDamageType(player, moveInputX, moveInputY);
                StartWeaponSwing(weaponSwing, player, swingType, swingSeconds * 0.7f);

                if (targetEnemy != nullptr) {
                    ResolveBumpAttack(player, *targetEnemy, actionMessage, swingType);
                }
            }
        }
        else if (showInventory) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                if (selectedItemIndex > 0) selectedItemIndex--;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                if (!player.inventory.empty() && selectedItemIndex < (int)player.inventory.size() - 1) {
                    selectedItemIndex++;
                }
            }
            if (IsKeyPressed(KEY_D)) {
                if (!player.inventory.empty() && selectedItemIndex < (int)player.inventory.size()) {
                    GroundItem dropped;
                    dropped.x = (int)(player.x + 0.5f); dropped.y = (int)(player.y + 0.5f);
                    dropped.item = player.inventory[selectedItemIndex];
                    groundItems.push_back(dropped);
                    player.inventory.erase(player.inventory.begin() + selectedItemIndex);
                    if (selectedItemIndex >= (int)player.inventory.size() && selectedItemIndex > 0) selectedItemIndex--;
                    actionMessage = "Dropped item.";
                }
            }
            if (IsKeyPressed(KEY_E)) {
                if (!player.inventory.empty() && selectedItemIndex < (int)player.inventory.size()) {
                    Item itemToEquip = player.inventory[selectedItemIndex];
                    EquipSlot targetSlot = SLOT_NONE;

                    if (itemToEquip.weaponTypeId >= 0) targetSlot = SLOT_MAIN_HAND;
                    else if (itemToEquip.ammoTypeId >= 0) targetSlot = SLOT_AMMO;
                    else if (!itemToEquip.archetypeId.empty()) {
                        for (const auto& arch : G_ITEM_ARCHETYPES) {
                            if (arch.id == itemToEquip.archetypeId) { targetSlot = arch.slot; break; }
                        }
                    }

                    if (targetSlot != SLOT_NONE && targetSlot < SLOT_SINGLE_COUNT) {
                        Item oldMainHand = player.equippedSlots[SLOT_MAIN_HAND];
                        Item oldOffHand = player.equippedSlots[SLOT_OFF_HAND];
                        Item oldItem = player.equippedSlots[targetSlot];
                        player.equippedSlots[targetSlot] = itemToEquip;
                        player.inventory.erase(player.inventory.begin() + selectedItemIndex);
                        if (!oldItem.IsEmpty()) player.inventory.push_back(oldItem);
                        if (selectedItemIndex >= (int)player.inventory.size() && selectedItemIndex > 0) selectedItemIndex--;
                        actionMessage = "Equipped item.";
                        SyncHotbarAfterEquipChange(player, oldMainHand, oldOffHand);
                    }
                }
            }
            if (IsKeyPressed(KEY_U)) {
                Item oldMainHand = player.equippedSlots[SLOT_MAIN_HAND];
                Item oldOffHand = player.equippedSlots[SLOT_OFF_HAND];
                for (int s = 0; s < SLOT_SINGLE_COUNT; s++) {
                    if (!player.equippedSlots[s].IsEmpty()) {
                        player.inventory.push_back(player.equippedSlots[s]);
                        player.equippedSlots[s] = Item();
                        actionMessage = "Unequipped item.";
                        SyncHotbarAfterEquipChange(player, oldMainHand, oldOffHand);
                        break;
                    }
                }
            }
        }

        if (!menuOpen)
        {
            auto isEnemyPositionFree = [&](float centerX, float centerY, float radius) -> bool {
                return IsWorldPositionWalkable(map, centerX, centerY, radius);
                };
            UpdateSquads(enemies, player);
            UpdateEnemyBehaviors(enemies, player, GetFrameTime(), isEnemyPositionFree, isOpaque, navMesh, rooms, floorAlert, actionMessage);
            if (player.hp <= 0)
            {
                player.hp = 0;
                showDeathScreen = true;
            }
        }

       if (!menuOpen && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !aimingAbilityId.empty())
       {
           aimingAbilityId = "";
           actionMessage = "Aim cancelled.";
       }
       else if (!menuOpen && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
       {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            int clickX = (int)floorf(mouseWorldPos.x / (float)tileSize);
            int clickY = (int)floorf(mouseWorldPos.y / (float)tileSize);
            float clickWorldX = mouseWorldPos.x / (float)tileSize;
            float clickWorldY = mouseWorldPos.y / (float)tileSize;
            bool foundSomething = false;

            float reachDeltaX = ((float)clickX + 0.5f) - (player.x + 0.5f);
            float reachDeltaY = ((float)clickY + 0.5f) - (player.y + 0.5f);
            bool inReach = (reachDeltaX * reachDeltaX + reachDeltaY * reachDeltaY) <= INTERACT_RANGE * INTERACT_RANGE;
            if (currentMode != MODE_LOOK && !inReach)
            {
                actionMessage = "Too far away.";
                foundSomething = true; // Skips every branch below
            }

            for (const auto& enemy : enemies) {
                float enemyClickDx = clickWorldX - (enemy.x + 0.5f);
                float enemyClickDy = clickWorldY - (enemy.y + 0.5f);
                bool clickedEnemy = (enemyClickDx * enemyClickDx + enemyClickDy * enemyClickDy) <= ENEMY_CLICK_RADIUS * ENEMY_CLICK_RADIUS;
                if (!foundSomething && clickedEnemy && (!enableFog || explored[clickX][clickY])) {
                    std::string enemyName = GetEnemyName(enemy);
                    if (enemy.isDead) {
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "The lifeless corpse of a " + enemyName + " lies here."; break;
                        case MODE_SPEAK: actionMessage = "You speak to the corpse, but it remains silent."; break;
                        case MODE_GRAB:  actionMessage = "You pull on the heavy corpse, but cannot carry it."; break;
                        case MODE_STEAL: actionMessage = "You search the body, but find nothing of value."; break;
                        }
                    }
                    else {
                        switch (currentMode) {
                        case MODE_LOOK: {
                            float hpPercent = (float)enemy.hp / enemy.maxHp;
                            if (hpPercent >= 1.0f) actionMessage = "The " + enemyName + " looks completely unharmed.";
                            else if (hpPercent >= 0.5f) actionMessage = "The " + enemyName + " has some cuts and bruises.";
                            else if (hpPercent >= 0.2f) actionMessage = "The " + enemyName + " looks severely wounded!";
                            else actionMessage = "The " + enemyName + " is clinging to life...";
                            actionMessage += " It seems " + GetEnemyIntentText(enemy) + " toward you.";
                            if (GetEnemyIntentText(enemy) == "hostile")
                            {
                                if (enemy.targetIsPlayer)
                                {
                                    actionMessage += " It has noticed you.";
                                }
                                else if (enemy.targetIndex >= 0)
                                {
                                    actionMessage += " It is busy with something else.";
                                }
                                else
                                {
                                    actionMessage += " It has not noticed you.";
                                }
                            }
                            break;
                        }
                        case MODE_SPEAK: actionMessage = "The " + enemyName + " growls hostility at you!"; break;
                        case MODE_GRAB:  actionMessage = "The " + enemyName + " snaps at your hand as you reach out!"; break;
                        case MODE_STEAL: actionMessage = "You reach for its pockets, but it swats your hand away!"; break;
                        }
                    }
                    foundSomething = true;
                    break;
                }
            }

            if (!foundSomething) {
                for (const auto& item : groundItems) {
                    if (item.x == clickX && item.y == clickY && (!enableFog || explored[clickX][clickY])) {
                        std::string itemName = GetGroundItemName(item.item);
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "You see a " + itemName + " lying on the ground."; break;
                        case MODE_SPEAK: actionMessage = "You talk to the " + itemName + ". It offers no reply."; break;
                        case MODE_GRAB:  actionMessage = "Press 'G' while standing over it to pick up the " + itemName + "."; break;
                        case MODE_STEAL: actionMessage = "It's on the floor�taking it is just picking it up."; break;
                        }
                        foundSomething = true;
                        break;
                    }
                }
            }

            if (!foundSomething && clickX == (int)(player.x + 0.5f) && clickY == (int)(player.y + 0.5f)) {
                switch (currentMode) {
                case MODE_LOOK:  actionMessage = "That's you (" + player.name + "). You're doing great."; break;
                case MODE_SPEAK: actionMessage = "Talking to yourself again?"; break;
                case MODE_GRAB:  actionMessage = "You give yourself a comforting hug."; break;
                case MODE_STEAL: actionMessage = "What have I got in my pocket?"; break;
                }
                foundSomething = true;
            }

            if (!foundSomething && clickX >= 0 && clickX < MAP_WIDTH && clickY >= 0 && clickY < MAP_HEIGHT) {
                if (!enableFog || explored[clickX][clickY]) {
                    TileType tile = map[clickX][clickY];
                    switch (tile) {
                    case TILE_WALL:
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "A solid, damp stone wall."; break;
                        case MODE_SPEAK: actionMessage = "You talk to the wall. Nothing happens."; break;
                        case MODE_GRAB:  actionMessage = "You feel the cold, rough surface of the stone."; break;
                        case MODE_STEAL: actionMessage = "You can't pocket a whole dungeon wall."; break;
                        }
                        break;
                    case TILE_STAIR_DOWN:
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "A set of dark stairs leading deeper down."; break;
                        case MODE_SPEAK: actionMessage = "Your voice echoes down the stairwell."; break;
                        case MODE_GRAB:  actionMessage = "You grip the dusty stone steps."; break;
                        case MODE_STEAL: actionMessage = "Try as you might, you can't steal the staircase."; break;
                        }
                        break;
                    case TILE_STAIR_UP:
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "Stairs leading back toward the upper floors."; break;
                        case MODE_SPEAK: actionMessage = "Your voice echoes upward."; break;
                        case MODE_GRAB:  actionMessage = "You grip the stair steps."; break;
                        case MODE_STEAL: actionMessage = "Try as you might, you can't steal the staircase."; break;
                        }
                        break;
                    case TILE_EXTRACT:
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "A ring of pale stones, faintly warm. A way out, if you have the nerve to take it."; break;
                        case MODE_SPEAK: actionMessage = "You call out. Somewhere far above, something answers."; break;
                        case MODE_GRAB:  actionMessage = "The stones are smooth and warm, worn by other hands."; break;
                        case MODE_STEAL: actionMessage = "You can't pocket a way out."; break;
                        }
                        break;
                    case TILE_FLOOR:
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "Bare, dusty dungeon floor."; break;
                        case MODE_SPEAK: actionMessage = "You talk to the floor tiles. They don't respond."; break;
                        case MODE_GRAB:  actionMessage = "You touch the cold floor."; break;
                        case MODE_STEAL: actionMessage = "There is nothing on this tile to steal."; break;
                        }
                        break;
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_F)) enableFog = !enableFog;
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
        if (IsKeyPressed(KEY_N)) showNavDebug = !showNavDebug;
        if (!IsPlayerOnTile(map, player, TILE_EXTRACT))
        {
            extractArmed = false;
        }

        float wheelMove = GetMouseWheelMove();
        if (wheelMove > 0) currentMode = (ActionMode)((currentMode + 1) % 4);
        else if (wheelMove < 0) currentMode = (ActionMode)((currentMode + 3) % 4);

        float targetX = (float)player.x * tileSize + (tileSize / 2.0f);
        float targetY = (float)player.y * tileSize + (tileSize / 2.0f);
        camera.target.x += (targetX - camera.target.x) * 10.0f * GetFrameTime();
        camera.target.y += (targetY - camera.target.y) * 10.0f * GetFrameTime();

        bool spaceUsedForExtract = false;
        if (IsKeyPressed(KEY_SPACE) && !menuOpen && IsPlayerOnTile(map, player, TILE_EXTRACT))
        {
            spaceUsedForExtract = true;
            if (!extractArmed)
            {
                extractArmed = true;
                actionMessage = "Press Space again to leave the dungeon. Your pack will be banked.";
            }
            else
            {
                extractArmed = false;
                int bankedCount = DepositInventoryToStash(player);
                AddToRoster(player);
                extractSummary = player.name + " returns to the tavern. Banked " + std::to_string(bankedCount) + " items, the stash now holds " + std::to_string((int)G_STASH.items.size()) + " stacks.";
                SaveMetaGame();
                showExtractScreen = true;
                continue;
            }
        }

        if (IsKeyPressed(KEY_SPACE) && !menuOpen && !spaceUsedForExtract) {
            // Same four-point circle check movement collision uses (see IsWorldPositionWalkable),
            // so "standing on the stair" means the same footprint here as it does for walls.
            float trueCenterX = player.x + 0.5f;
            float trueCenterY = player.y + 0.5f;
            float stairCheckPoints[4][2] = {
                { trueCenterX - PLAYER_COLLISION_RADIUS, trueCenterY },
                { trueCenterX + PLAYER_COLLISION_RADIUS, trueCenterY },
                { trueCenterX, trueCenterY - PLAYER_COLLISION_RADIUS },
                { trueCenterX, trueCenterY + PLAYER_COLLISION_RADIUS }
            };

            bool wentDown = false;
            bool wentUp = false;
            for (int i = 0; i < 4; i++) {
                int tileX = (int)stairCheckPoints[i][0];
                int tileY = (int)stairCheckPoints[i][1];
                if (tileX < 0 || tileX >= MAP_WIDTH || tileY < 0 || tileY >= MAP_HEIGHT) {
                    continue;
                }
                if (map[tileX][tileY] == TILE_STAIR_DOWN) {
                    wentDown = true;
                }
                if (map[tileX][tileY] == TILE_STAIR_UP && currentFloor > 0) {
                    wentUp = true;
                }
            }

            if (wentDown || wentUp) {
                PerformFloorTransition(wentDown);

                for (int x = 0; x < MAP_WIDTH; x++) {
                    for (int y = 0; y < MAP_HEIGHT; y++) {
                        dungeon[currentFloor].savedMap[x][y] = map[x][y];
                        dungeon[currentFloor].savedExplored[x][y] = explored[x][y];
                    }
                }
                dungeon[currentFloor].savedEnemies = enemies;
                dungeon[currentFloor].savedItems = groundItems;
                dungeon[currentFloor].alertLevel = floorAlert;

                if (wentDown) currentFloor++;
                if (wentUp) currentFloor--;

                if (currentFloor >= (int)dungeon.size()) {
                    GenerateFloor(map, explored, rooms, player, enemies, BuildFloorParams(currentExpedition, activeModifierIds, currentFloor + 1));
                    groundItems.clear();
                    floorAlert = 0;
                    LevelState newFloor;
                    dungeon.push_back(newFloor);
                }
                else {
                    for (int x = 0; x < MAP_WIDTH; x++) {
                        for (int y = 0; y < MAP_HEIGHT; y++) {
                            map[x][y] = dungeon[currentFloor].savedMap[x][y];
                            explored[x][y] = dungeon[currentFloor].savedExplored[x][y];
                        }
                    }
                    enemies = dungeon[currentFloor].savedEnemies;
                    ResolveOffscreenFactionConflicts(enemies, currentFloor + 1);
                    floorAlert = dungeon[currentFloor].alertLevel;
                    groundItems = dungeon[currentFloor].savedItems;

                    TileType targetStair = wentDown ? TILE_STAIR_UP : TILE_STAIR_DOWN;
                    for (int x = 0; x < MAP_WIDTH; x++) {
                        for (int y = 0; y < MAP_HEIGHT; y++) {
                            if (map[x][y] == targetStair) {
                                player.x = x + 1;
                                player.y = y;
                            }
                        }
                    }
                }
                rebuildNavMesh();
                continue;
            }
        }

        int visionRadius = PLAYER_VISION_RADIUS + playerVisionMod;
        if (visionRadius < 2)
        {
            visionRadius = 2;
        }
        for (int i = -visionRadius; i <= visionRadius; i++) {
            for (int j = -visionRadius; j <= visionRadius; j++) {
                int playerCenterTileX = (int)(player.x + 0.5f);
                int playerCenterTileY = (int)(player.y + 0.5f);
                int viewX = playerCenterTileX + i;
                int viewY = playerCenterTileY + j;
                if (viewX < 0 || viewX >= MAP_WIDTH || viewY < 0 || viewY >= MAP_HEIGHT) {
                    continue;
                }
                if (i * i + j * j > visionRadius * visionRadius) {
                    continue;
                }
                if (HasLineOfSight(playerCenterTileX, playerCenterTileY, viewX, viewY, isOpaque)) {
                    explored[viewX][viewY] = true;
                }
            }
        }

        // --- DRAW GAMEPLAY ---
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

        // Only walk tiles the camera can see. Draw-only: map and explored are untouched.
        Vector2 viewTopLeft = GetScreenToWorld2D({ 0.0f, 0.0f }, camera);
        Vector2 viewBottomRight = GetScreenToWorld2D({ (float)GetScreenWidth(), (float)GetScreenHeight() }, camera);
        int firstTileX = std::max(-30, (int)floorf(viewTopLeft.x / (float)tileSize) - 1);
        int lastTileX = std::min(MAP_WIDTH + 29, (int)floorf(viewBottomRight.x / (float)tileSize) + 1);
        int firstTileY = std::max(-30, (int)floorf(viewTopLeft.y / (float)tileSize) - 1);
        int lastTileY = std::min(MAP_HEIGHT + 29, (int)floorf(viewBottomRight.y / (float)tileSize) + 1);

        for (int x = firstTileX; x <= lastTileX; x++) {
            for (int y = firstTileY; y <= lastTileY; y++) {
                if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
                    int edgeX = std::max(0, std::min(x, MAP_WIDTH - 1));
                    int edgeY = std::max(0, std::min(y, MAP_HEIGHT - 1));
                    bool fakeExplored = explored[edgeX][edgeY] && (std::abs(x - edgeX) <= 2) && (std::abs(y - edgeY) <= 2);
                    if (fakeExplored || !enableFog) {
                        DrawText("#", x * tileSize + 4, y * tileSize + 2, tileSize, DARKGRAY);
                    }
                    continue;
                }
                if (explored[x][y] || !enableFog) {
                    if (map[x][y] == TILE_WALL) DrawText("#", x * tileSize + 4, y * tileSize + 2, tileSize, DARKGRAY);
                    else if (map[x][y] == TILE_FLOOR) DrawText(".", x * tileSize + 8, y * tileSize + 2, tileSize, ColorAlpha(DARKGRAY, 0.5f));
                    else if (map[x][y] == TILE_STAIR_UP) DrawText("<", x * tileSize + 4, y * tileSize + 2, tileSize, YELLOW);
                    else if (map[x][y] == TILE_STAIR_DOWN) DrawText(">", x * tileSize + 4, y * tileSize + 2, tileSize, YELLOW);
                    else if (map[x][y] == TILE_EXTRACT) DrawText("X", x * tileSize + 4, y * tileSize + 2, tileSize, SKYBLUE);
                }
            }
        }

        if (!aimingAbilityId.empty()) {
            const AbilityDef* aimingAbility = FindAbility(aimingAbilityId);
            if (aimingAbility != nullptr) {
                Vector2 aimWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
                float aimTileX = aimWorldPos.x / (float)tileSize;
                float aimTileY = aimWorldPos.y / (float)tileSize;
                DrawTargetingOverlay(player.x + 0.5f, player.y + 0.5f, aimTileX, aimTileY, *aimingAbility, tileSize);
            }
        }

        for (const auto& item : groundItems) {
            if (!enableFog || explored[item.x][item.y]) {
                DrawText("?", item.x * tileSize + 6, item.y * tileSize + 2, tileSize, GOLD);
            }
        }

        for (const auto& enemy : enemies) {
            if (!enableFog || explored[(int)(enemy.x + 0.5f)][(int)(enemy.y + 0.5f)]) {
                const char* enemyChar = TextFormat("%c", enemy.symbol);
                Color drawColor = enemy.isDead ? GRAY : enemy.color;
                float rotation = enemy.isDead ? 90.0f : 0.0f;
                Vector2 textSize = MeasureTextEx(GetFontDefault(), enemyChar, (float)tileSize, 1.0f);
                Vector2 origin = { textSize.x / 2.0f, textSize.y / 2.0f };
                Vector2 position = { enemy.x * tileSize + tileSize / 2.0f, enemy.y * tileSize + tileSize / 2.0f };
                DrawTextPro(GetFontDefault(), enemyChar, position, origin, rotation, (float)tileSize, 1.0f, drawColor);
            }
        }

        if (showNavDebug) {
            for (const auto& enemy : enemies) {
                if (!enemy.hasSquadSlot) {
                    continue;
                }
                Vector2 slotPos = { enemy.squadSlotX * tileSize + tileSize / 2.0f, enemy.squadSlotY * tileSize + tileSize / 2.0f };
                Vector2 enemyPos = { enemy.x * tileSize + tileSize / 2.0f, enemy.y * tileSize + tileSize / 2.0f };
                DrawCircleV(slotPos, 5.0f, YELLOW);
                DrawLineV(enemyPos, slotPos, Fade(YELLOW, 0.5f));
            }
        }

        if (showNavDebug) {
            for (size_t e = 0; e < enemies.size(); e++) {
                const Enemy& debugEnemy = enemies[e];
                if (debugEnemy.isDead) {
                    continue;
                }
                Vector2 fromPos = { debugEnemy.x * tileSize + tileSize / 2.0f, debugEnemy.y * tileSize + tileSize / 2.0f };
                if (debugEnemy.targetIsPlayer) {
                    Vector2 toPos = { player.x * tileSize + tileSize / 2.0f, player.y * tileSize + tileSize / 2.0f };
                    DrawLineV(fromPos, toPos, Fade(RED, 0.6f));
                }
                else if (debugEnemy.targetIndex >= 0 && debugEnemy.targetIndex < (int)enemies.size()) {
                    const Enemy& debugTarget = enemies[debugEnemy.targetIndex];
                    Vector2 toPos = { debugTarget.x * tileSize + tileSize / 2.0f, debugTarget.y * tileSize + tileSize / 2.0f };
                    DrawLineV(fromPos, toPos, Fade(ORANGE, 0.6f));
                }
                DrawText(debugEnemy.behaviorId.c_str(), (int)fromPos.x - 14, (int)fromPos.y - 22, 10, WHITE);
            }
        }

        DrawText("@", player.x* tileSize + 4, player.y* tileSize + 2, tileSize, GREEN);
        DrawWeaponVisual(weaponSwing, player, tileSize);
        if (showNavDebug) {
            navMesh.DrawDebug(tileSize);
        }
        EndMode2D();
        if (currentExpedition != nullptr)
        {
            FloorParams hudFloor = BuildFloorParams(currentExpedition, activeModifierIds, currentFloor + 1);
            std::string floorText = currentExpedition->name + "  |  Floor " + std::to_string(hudFloor.floorNumber);
            floorText += "  |  Chunk " + std::to_string(hudFloor.chunkIndex + 1) + "  (" + std::to_string(hudFloor.floorInChunk) + "/" + std::to_string(FLOORS_PER_CHUNK) + ")";
            for (size_t m = 0; m < hudFloor.modifiers.size(); m++)
            {
                floorText += "  |  " + hudFloor.modifiers[m]->name;
            }
            if (hudFloor.dangerRating > 0)
            {
                floorText += "  |  Danger +" + std::to_string(hudFloor.dangerRating);
            }
            float floorTextScale = GetUIScale();
            DrawText(floorText.c_str(), (int)(18 * floorTextScale), (int)(18 * floorTextScale), (int)(22 * floorTextScale), GOLD);
        }
        DrawHotbar(player);

        // HUD / UI
        float hudScale = GetUIScale();
        int hudPanelW = (int)(340 * hudScale);
        int hudPanelH = (int)(200 * hudScale);
        int hudX = GetScreenWidth() - hudPanelW - (int)(20 * hudScale);
        int hudY = (int)(20 * hudScale);
        DrawRectangle(hudX - (int)(10 * hudScale), hudY, hudPanelW, hudPanelH, Fade(BLACK, 0.85f));

        DrawText(TextFormat("%s (%s)", player.name.c_str(), player.className.c_str()), hudX, hudY + (int)(8 * hudScale), (int)(24 * hudScale), GOLD);

        Color alertColor = GRAY;
        std::string alertLabel = "CALM";
        if (IsFloorSearching(floorAlert)) {
            alertColor = RED;
            alertLabel = "SEARCHING";
        }
        else if (IsFloorWary(floorAlert)) {
            alertColor = ORANGE;
            alertLabel = "WARY";
        }
        DrawText(TextFormat("ALERT: %s (%d)", alertLabel.c_str(), floorAlert), hudX, hudY + (int)(72 * hudScale), (int)(16 * hudScale), alertColor);

        Color modeColor = WHITE; const char* modeSymbol = "";
        switch (currentMode) {
        case MODE_LOOK:  modeColor = SKYBLUE; modeSymbol = "?"; break;
        case MODE_SPEAK: modeColor = GOLD;    modeSymbol = "\""; break;
        case MODE_GRAB:  modeColor = GREEN;   modeSymbol = "m"; break;
        case MODE_STEAL: modeColor = PURPLE;  modeSymbol = "$"; break;
        }

        int modeRowY = hudY + (int)(44 * hudScale);
        int modeBoxSize = (int)(28 * hudScale);
        DrawRectangle(hudX, modeRowY, modeBoxSize, modeBoxSize, modeColor);
        DrawText(modeSymbol, hudX + (int)(modeBoxSize * 0.3f), modeRowY + (int)(modeBoxSize * 0.15f), (int)(20 * hudScale), BLACK);
        DrawText(TextFormat("MODE: %s", modeNames[currentMode]), hudX + modeBoxSize + (int)(10 * hudScale), modeRowY + (int)(4 * hudScale), (int)(22 * hudScale), modeColor);

        int barW = hudPanelW - (int)(20 * hudScale);
        int barH = (int)(24 * hudScale);
        int barFont = (int)(20 * hudScale);

        int barY = hudY + (int)(90 * hudScale);
        DrawRectangle(hudX, barY, barW, barH, DARKGRAY);
        DrawRectangle(hudX, barY, (player.hp * barW) / std::max(1, player.maxHp), barH, RED);
        DrawText(TextFormat("HP: %i/%i", player.hp, player.maxHp), hudX + (int)(6 * hudScale), barY + (int)(2 * hudScale), barFont, WHITE);

        barY += (int)(32 * hudScale);
        DrawRectangle(hudX, barY, barW, barH, DARKGRAY);
        DrawRectangle(hudX, barY, (player.stamina * barW) / std::max(1, player.maxStamina), barH, GREEN);
        DrawText(TextFormat("SP: %i/%i", player.stamina, player.maxStamina), hudX + (int)(6 * hudScale), barY + (int)(2 * hudScale), barFont, WHITE);

        barY += (int)(32 * hudScale);
        DrawRectangle(hudX, barY, barW, barH, DARKGRAY);
        DrawRectangle(hudX, barY, (player.mana * barW) / std::max(1, player.maxMana), barH, BLUE);
        DrawText(TextFormat("MP: %i/%i", player.mana, player.maxMana), hudX + (int)(6 * hudScale), barY + (int)(2 * hudScale), barFont, WHITE);

        if (!aimingAbilityId.empty()) {
            const AbilityDef* hudAimAbility = FindAbility(aimingAbilityId);
            if (hudAimAbility != nullptr) {
                std::string aimText = "AIMING: " + hudAimAbility->name + " (Esc or right click cancels)";
                DrawText(aimText.c_str(), hudX, barY + (int)(36 * hudScale), (int)(18 * hudScale), RED);
            }
        }

        if (!actionMessage.empty()) {
            float msgScale = GetUIScale();
            int msgFontSize = (int)(26 * msgScale);
            int msgWidth = MeasureText(actionMessage.c_str(), msgFontSize);
            DrawRectangle((GetScreenWidth() - msgWidth) / 2 - (int)(12 * msgScale), GetScreenHeight() - (int)(60 * msgScale), msgWidth + (int)(24 * msgScale), (int)(40 * msgScale), Fade(BLACK, 0.8f));
            DrawText(actionMessage.c_str(), (GetScreenWidth() - msgWidth) / 2, GetScreenHeight() - (int)(52 * msgScale), msgFontSize, RAYWHITE);
        }

        if (showMenuHub) {
            float hubScale = GetUIScale();
            int panelW = (int)(GetScreenWidth() * 0.82f);
            int panelH = (int)(GetScreenHeight() * 0.82f);
            int invX = GetScreenWidth() / 2 - panelW / 2;
            int invY = GetScreenHeight() / 2 - panelH / 2;
            DrawRectangle(invX, invY, panelW, panelH, Fade(BLACK, 0.9f));
            DrawRectangleLines(invX, invY, panelW, panelH, GOLD);
            DrawMenuTabBar(invX, invY, panelW, currentMenuTab);

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                Vector2 tabMouse = GetMousePosition();
                int clickedTab = GetMenuTabAtPoint(invX, invY, panelW, (int)tabMouse.x, (int)tabMouse.y);
                if (clickedTab >= 0)
                {
                    currentMenuTab = (MenuTab)clickedTab;
                }
            }

            int tabBarHeight = GetMenuTabBarHeight();
            int footerHeight = GetMenuFooterHeight();
            int contentY = invY + tabBarHeight + (int)(24 * hubScale);
            int contentBottom = invY + panelH - footerHeight;
            int lineFont = (int)(24 * hubScale);
            int smallFont = (int)(20 * hubScale);

            if (currentMenuTab == MENU_TAB_INVENTORY) {
                DrawText("EQUIPPED:", invX + panelW / 2 + (int)(24 * hubScale), contentY, lineFont, GOLD);
                int slotDrawY = contentY + (int)(lineFont * 1.5f);
                for (int s = 0; s < SLOT_SINGLE_COUNT; s++) {
                    if (!player.equippedSlots[s].IsEmpty()) {
                        std::string eqName = GetGroundItemName(player.equippedSlots[s]);
                        DrawText(eqName.c_str(), invX + panelW / 2 + (int)(24 * hubScale), slotDrawY, smallFont, WHITE);
                        slotDrawY += (int)(smallFont * 1.5f);
                    }
                }

                if (player.inventory.empty()) {
                    DrawText("Your inventory is empty.", invX + (int)(24 * hubScale), contentY, lineFont, LIGHTGRAY);
                }
                else {
                    int rowHeight = (int)(38 * hubScale);
                    for (size_t i = 0; i < player.inventory.size(); i++) {
                        int lineY = contentY + (int)i * rowHeight;
                        if (lineY > contentBottom) {
                            break;
                        }
                        std::string itemName = GetGroundItemName(player.inventory[i]);
                        Color itemColor = WHITE;
                        if ((int)i == selectedItemIndex) {
                            itemColor = YELLOW;
                            DrawText(">", invX + (int)(18 * hubScale), lineY, lineFont, YELLOW);
                        }
                        std::string line = itemName + " x" + std::to_string(player.inventory[i].quantity);
                        DrawText(line.c_str(), invX + (int)(48 * hubScale), lineY, lineFont, itemColor);
                    }
                }

                DrawText("E: Equip   D: Drop   U: Unequip", invX + (int)(24 * hubScale), invY + panelH - footerHeight + (int)(10 * hubScale), smallFont, GRAY);
            }
            else if (currentMenuTab == MENU_TAB_ABILITIES) {
                DrawAbilitiesTab(invX, invY, panelW, panelH, player);
            }
            else if (currentMenuTab == MENU_TAB_MAGIC) {
                DrawMagicTab(invX, invY, panelW, panelH);
            }
            else {
                DrawCharacterTab(invX, invY, panelW, panelH, player);
            }

            DrawText("Left/Right or [ / ] to switch tabs   |   Tab to close", invX + (int)(24 * hubScale), invY + panelH - (int)(34 * hubScale), smallFont, GRAY);
        }

        if (showPauseMenu) {
            float pauseScale = GetUIScale();
            int menuWidth = (int)(460 * pauseScale); int menuHeight = (int)(400 * pauseScale);
            int menuX = GetScreenWidth() / 2 - menuWidth / 2;
            int menuY = GetScreenHeight() / 2 - menuHeight / 2;
            DrawRectangle(menuX, menuY, menuWidth, menuHeight, Fade(BLACK, 0.95f));
            DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);
            int titleFont = (int)(30 * pauseScale);
            DrawText("PAUSED", menuX + (menuWidth - MeasureText("PAUSED", titleFont)) / 2, menuY + (int)(24 * pauseScale), titleFont, GOLD);

            const char* pauseOptions[] = { "Resume", "Save Game", "Load Game", "Options", "Abandon Run", "Quit (abandons run)" };
            int optionFont = (int)(26 * pauseScale);
            int rowHeight = (int)(48 * pauseScale);
            Vector2 mousePos = GetMousePosition();
            for (int i = 0; i < 6; i++) {
                int optY = menuY + (int)(90 * pauseScale) + i * rowHeight;
                Rectangle optRect = { (float)menuX + (60 * pauseScale), (float)optY - (4 * pauseScale), (float)menuWidth - (120 * pauseScale), (float)(rowHeight - 8 * pauseScale) };
                if (CheckCollisionPointRec(mousePos, optRect)) {
                    pauseMenuSelection = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (i == 0) showPauseMenu = false;
                        else if (i == 1) { actionMessage = "Save feature not implemented yet."; showPauseMenu = false; }
                        else if (i == 2) { actionMessage = "Load feature not implemented yet."; showPauseMenu = false; }
                        else if (i == 3) { actionMessage = "Options feature not implemented yet."; showPauseMenu = false; }
                        else if (i == 4) {
                            ResetTitleScreen();
                            currentState = STATE_TITLE;
                            showPauseMenu = false;
                        }
                        else if (i == 5) keepRunning = false;
                    }
                }

                Color optColor = (i == pauseMenuSelection) ? YELLOW : WHITE;
                if (i == pauseMenuSelection) DrawText(">", menuX + (int)(70 * pauseScale), optY, optionFont, YELLOW);
                DrawText(pauseOptions[i], menuX + (int)(105 * pauseScale), optY, optionFont, optColor);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
