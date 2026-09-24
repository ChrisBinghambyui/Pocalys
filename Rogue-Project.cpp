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
#include "FeatData.h"
#include <string>

const int MAP_WIDTH = 80;
const int MAP_HEIGHT = 45;
const int MAX_ROOMS = 15;

enum GameState {
    STATE_TITLE,
    STATE_TAVERN,
    STATE_GAMEPLAY
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
    else if (!item.archetypeId.empty()) {
        for (const auto& arch : G_ITEM_ARCHETYPES) {
            if (arch.id == item.archetypeId) return arch.name;
        }
        return item.archetypeId;
    }
    return "item";
}

void GenerateFloor(TileType map[MAP_WIDTH][MAP_HEIGHT], bool explored[MAP_WIDTH][MAP_HEIGHT], std::vector<Room>& rooms, Player& player, std::vector<Enemy>& enemies, int floorNumber) {
    rooms.clear();
    for (int x = 0; x < MAP_WIDTH; x++) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            map[x][y] = TILE_WALL;
            explored[x][y] = false;
        }
    }

    int attempts = 0;
    while (rooms.size() < MAX_ROOMS && attempts < 100) {
        attempts++;
        int rw = GetRandomValue(4, 8), rh = GetRandomValue(4, 8);
        int rx = GetRandomValue(1, MAP_WIDTH - rw - 1), ry = GetRandomValue(1, MAP_HEIGHT - rh - 1);
        Room nr = { rx, ry, rw, rh };

        bool failed = false;
        for (const Room& orm : rooms) { if (nr.intersects(orm)) { failed = true; break; } }
        if (failed) continue;

        for (int x = nr.x; x < nr.x + nr.width; x++) {
            for (int y = nr.y; y < nr.y + nr.height; y++) map[x][y] = TILE_FLOOR;
        }

        if (!rooms.empty()) {
            int pcx = rooms.back().centerX(), pcy = rooms.back().centerY();
            int ncx = nr.centerX(), ncy = nr.centerY();
            for (int x = std::min(pcx, ncx); x <= std::max(pcx, ncx); x++) map[x][pcy] = TILE_FLOOR;
            for (int y = std::min(pcy, ncy); y <= std::max(pcy, ncy); y++) map[ncx][y] = TILE_FLOOR;
        }
        rooms.push_back(nr);
    }

    map[rooms[0].centerX()][rooms[0].centerY()] = TILE_STAIR_UP;

    int furthestIdx = 0; float maxDist = 0;
    for (size_t i = 1; i < rooms.size(); i++) {
        float dist = std::pow(rooms[i].centerX() - rooms[0].centerX(), 2) + std::pow(rooms[i].centerY() - rooms[0].centerY(), 2);
        if (dist > maxDist) { maxDist = dist; furthestIdx = i; }
    }
    map[rooms[furthestIdx].centerX()][rooms[furthestIdx].centerY()] = TILE_STAIR_DOWN;

    player.x = rooms[0].centerX() + 1;
    player.y = rooms[0].centerY();

    enemies.clear();
    for (size_t i = 1; i < rooms.size(); i++) {
        const EnemyArchetype* archetype = PickSpawnArchetype(floorNumber);
        if (archetype == nullptr)
        {
            continue;
        }
        enemies.push_back(CreateEnemy(*archetype, floorNumber, rooms[i].centerX(), rooms[i].centerY()));
    }
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

    TileType map[MAP_WIDTH][MAP_HEIGHT];
    bool explored[MAP_WIDTH][MAP_HEIGHT] = { false };
    std::vector<Room> rooms;
    std::vector<LevelState> dungeon;
    int currentFloor = 0;
    std::vector<Enemy> enemies;
    std::vector<GroundItem> groundItems;
    std::string actionMessage = "";

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

            for (int k = 0; k < 6; k++) {
                if (IsKeyPressed(KEY_ONE + k)) selectedCandidate = k;
            }

            bool confirmSelection = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);

            BeginDrawing();
            ClearBackground(Color{ 18, 12, 14, 255 });

            DrawText("THE RUSTY ANVIL TAVERN", GetScreenWidth() / 2 - MeasureText("THE RUSTY ANVIL TAVERN", 32) / 2, 30, 32, GOLD);
            DrawText("Select a patron to descend into the dungeon...", GetScreenWidth() / 2 - MeasureText("Select a patron to descend into the dungeon...", 18) / 2, 70, 18, LIGHTGRAY);

            Vector2 mousePos = GetMousePosition();
            int cardW = 340; int cardH = 220;
            int marginX = 25; int marginY = 20;
            int startX = GetScreenWidth() / 2 - (cardW * 3 + marginX * 2) / 2;
            int startY = 110;

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
                DrawRectangleLinesEx(cardRect, isSelected ? 3.0f : 1.0f, borderCol);

                DrawText(TextFormat("%d. %s", i + 1, cand.name.c_str()), cx + 12, cy + 12, 18, isSelected ? YELLOW : WHITE);
                DrawText(TextFormat("%s  |  Birthsign: %s", cand.className.c_str(), cand.birthsign.c_str()), cx + 12, cy + 36, 14, ORANGE);

                DrawText(TextFormat("HP: %d  SP: %d  MP: %d", cand.maxHp, cand.maxStamina, cand.maxMana), cx + 12, cy + 58, 14, GREEN);
                DrawText(TextFormat("STR:%d END:%d AGI:%d INT:%d", cand.str, cand.end, cand.agi, cand.intel), cx + 12, cy + 78, 13, LIGHTGRAY);
                DrawText(TextFormat("WIL:%d PER:%d LCK:%d", cand.wil, cand.per, cand.lck), cx + 12, cy + 96, 13, LIGHTGRAY);

                std::string mainHand = "Bare Fists";
                if (!cand.equippedSlots[SLOT_MAIN_HAND].IsEmpty()) {
                    mainHand = GetGroundItemName(cand.equippedSlots[SLOT_MAIN_HAND]);
                }

                const RaceData* candRace = FindRace(cand.raceId);
                if (candRace != nullptr)
                {
                    DrawText(TextFormat("Race: %s", candRace->name.c_str()), cx + 12, cy + 140, 14, VIOLET);
                }
            }

            // Detail panel for the highlighted patron (mouse hover or keyboard selection)
            if (selectedCandidate >= 0 && selectedCandidate < (int)tavernCandidates.size())
            {
                const CharacterProfile& sel = tavernCandidates[selectedCandidate];
                const RaceData* selRace = FindRace(sel.raceId);
                int panelX = startX;
                int panelY = startY + 2 * (cardH + marginY) + 5;
                int panelW = cardW * 3 + marginX * 2;
                int panelH = 140;

                DrawRectangle(panelX, panelY, panelW, panelH, Color{ 28, 22, 24, 255 });
                DrawRectangleLinesEx(Rectangle{ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 1.0f, GOLD);

                if (selRace != nullptr)
                {
                    DrawText(("RACE: " + selRace->name).c_str(), panelX + 15, panelY + 12, 18, VIOLET);
                    DrawText(selRace->description.c_str(), panelX + 15, panelY + 36, 14, LIGHTGRAY);

                    const Feat* racialFeat = FindFeat(selRace->racialFeatId);
                    if (racialFeat != nullptr)
                    {
                        DrawText(racialFeat->name.c_str(), panelX + 15, panelY + 64, 16, ORANGE);
                        DrawText(racialFeat->description.c_str(), panelX + 15, panelY + 86, 14, LIGHTGRAY);
                    }

                    std::string bonusText = "SKILL BONUSES:  " + DescribeSkillBonuses(sel.raceSkillBonuses);
                    DrawText(bonusText.c_str(), panelX + 15, panelY + 114, 14, SKYBLUE);
                }
            }

            std::string hint = "[WASD / Arrows / 1-6] Select  |  [ENTER / Click] Begin Quest  |  [R] Reroll Patrons";
            DrawText(hint.c_str(), GetScreenWidth() / 2 - MeasureText(hint.c_str(), 18) / 2, GetScreenHeight() - 50, 18, GOLD);

            EndDrawing();

            if (confirmSelection) {
                ApplyProfileToPlayer(tavernCandidates[selectedCandidate], player);

                dungeon.clear();
                currentFloor = 0;
                groundItems.clear();
                GenerateFloor(map, explored, rooms, player, enemies, currentFloor + 1);

                LevelState firstFloor;
                for (int x = 0; x < MAP_WIDTH; x++) {
                    for (int y = 0; y < MAP_HEIGHT; y++) {
                        firstFloor.savedMap[x][y] = map[x][y];
                        firstFloor.savedExplored[x][y] = explored[x][y];
                    }
                }
                firstFloor.savedEnemies = enemies;
                firstFloor.savedItems = groundItems;
                dungeon.push_back(firstFloor);

                camera.target = { (float)player.x * tileSize + (tileSize / 2.0f), (float)player.y * tileSize + (tileSize / 2.0f) };
                actionMessage = "You descend into the catacombs as " + player.name + " the " + player.className + ".";
                currentState = STATE_GAMEPLAY;
            }
            continue;
        }

        // --- GAMEPLAY INPUT & UPDATE ---
        camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        int nextX = player.x; int nextY = player.y;

        if (IsKeyPressed(KEY_ESCAPE)) {
            showPauseMenu = !showPauseMenu;
            if (showPauseMenu) showInventory = false;
        }

        if (IsKeyPressed(KEY_I) && !showPauseMenu) {
            showInventory = !showInventory;
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
        else if (!showInventory) {
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) nextX++;
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) nextX--;
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) nextY--;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) nextY++;

            if (IsKeyPressed(KEY_G)) {
                for (auto it = groundItems.begin(); it != groundItems.end(); ++it) {
                    if (it->x == player.x && it->y == player.y) {
                        player.inventory.push_back(it->item);
                        actionMessage = "Picked up item.";
                        groundItems.erase(it);
                        break;
                    }
                }
            }
        }
        else {
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
                    dropped.x = player.x; dropped.y = player.y;
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
                    else if (!itemToEquip.archetypeId.empty()) {
                        for (const auto& arch : G_ITEM_ARCHETYPES) {
                            if (arch.id == itemToEquip.archetypeId) { targetSlot = arch.slot; break; }
                        }
                    }

                    if (targetSlot != SLOT_NONE && targetSlot < SLOT_SINGLE_COUNT) {
                        Item oldItem = player.equippedSlots[targetSlot];
                        player.equippedSlots[targetSlot] = itemToEquip;
                        player.inventory.erase(player.inventory.begin() + selectedItemIndex);
                        if (!oldItem.IsEmpty()) player.inventory.push_back(oldItem);
                        if (selectedItemIndex >= (int)player.inventory.size() && selectedItemIndex > 0) selectedItemIndex--;
                        actionMessage = "Equipped item.";
                    }
                }
            }
            if (IsKeyPressed(KEY_U)) {
                for (int s = 0; s < SLOT_SINGLE_COUNT; s++) {
                    if (!player.equippedSlots[s].IsEmpty()) {
                        player.inventory.push_back(player.equippedSlots[s]);
                        player.equippedSlots[s] = Item();
                        actionMessage = "Unequipped item.";
                        break;
                    }
                }
            }
        }

        if (nextX != player.x || nextY != player.y) {
            bool enemyBlocking = false;
            for (auto it = enemies.begin(); it != enemies.end(); ++it) {
                if (it->x == nextX && it->y == nextY && !it->isDead) {
                    enemyBlocking = true;
                    ResolveBumpAttack(player, *it, actionMessage);
                    break;
                }
            }

            if (!enableFog || (!enemyBlocking && (map[nextX][nextY] == TILE_FLOOR || map[nextX][nextY] == TILE_STAIR_UP || map[nextX][nextY] == TILE_STAIR_DOWN))) {
                player.x = nextX;
                player.y = nextY;
                if (!enemyBlocking) actionMessage = "";
            }
        }

        if (!showPauseMenu && !showInventory && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            int clickX = (int)floorf(mouseWorldPos.x / (float)tileSize);
            int clickY = (int)floorf(mouseWorldPos.y / (float)tileSize);
            bool foundSomething = false;

            for (const auto& enemy : enemies) {
                if (enemy.x == clickX && enemy.y == clickY && (!enableFog || explored[clickX][clickY])) {
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
                        case MODE_STEAL: actionMessage = "It's on the floor—taking it is just picking it up."; break;
                        }
                        foundSomething = true;
                        break;
                    }
                }
            }

            if (!foundSomething && clickX == player.x && clickY == player.y) {
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
                        case MODE_STEAL: actionMessage = "You can't steal the staircase."; break;
                        }
                        break;
                    case TILE_STAIR_UP:
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "Stairs leading back toward the upper floors."; break;
                        case MODE_SPEAK: actionMessage = "Your voice echoes upward."; break;
                        case MODE_GRAB:  actionMessage = "You grip the stair steps."; break;
                        case MODE_STEAL: actionMessage = "You can't steal the staircase."; break;
                        }
                        break;
                    case TILE_FLOOR:
                        switch (currentMode) {
                        case MODE_LOOK:  actionMessage = "Bare, dusty dungeon floor."; break;
                        case MODE_SPEAK: actionMessage = "You talk to the floor tiles. Silence."; break;
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

        float wheelMove = GetMouseWheelMove();
        if (wheelMove > 0) currentMode = (ActionMode)((currentMode + 1) % 4);
        else if (wheelMove < 0) currentMode = (ActionMode)((currentMode + 3) % 4);

        float targetX = (float)player.x * tileSize + (tileSize / 2.0f);
        float targetY = (float)player.y * tileSize + (tileSize / 2.0f);
        camera.target.x += (targetX - camera.target.x) * 10.0f * GetFrameTime();
        camera.target.y += (targetY - camera.target.y) * 10.0f * GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            bool wentDown = (map[player.x][player.y] == TILE_STAIR_DOWN);
            bool wentUp = (map[player.x][player.y] == TILE_STAIR_UP && currentFloor > 0);

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

                if (wentDown) currentFloor++;
                if (wentUp) currentFloor--;

                if (currentFloor >= (int)dungeon.size()) {
                    GenerateFloor(map, explored, rooms, player, enemies, currentFloor + 1);
                    groundItems.clear();
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
                continue;
            }
        }

        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                int viewX = player.x + i;
                int viewY = player.y + j;
                if (viewX >= 0 && viewX < MAP_WIDTH && viewY >= 0 && viewY < MAP_HEIGHT) {
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
                }
            }
        }

        for (const auto& item : groundItems) {
            if (!enableFog || explored[item.x][item.y]) {
                DrawText("?", item.x * tileSize + 6, item.y * tileSize + 2, tileSize, GOLD);
            }
        }

        for (const auto& enemy : enemies) {
            if (!enableFog || explored[enemy.x][enemy.y]) {
                const char* enemyChar = TextFormat("%c", enemy.symbol);
                Color drawColor = enemy.isDead ? GRAY : enemy.color;
                float rotation = enemy.isDead ? 90.0f : 0.0f;
                Vector2 textSize = MeasureTextEx(GetFontDefault(), enemyChar, (float)tileSize, 1.0f);
                Vector2 origin = { textSize.x / 2.0f, textSize.y / 2.0f };
                Vector2 position = { enemy.x * tileSize + tileSize / 2.0f, enemy.y * tileSize + tileSize / 2.0f };
                DrawTextPro(GetFontDefault(), enemyChar, position, origin, rotation, (float)tileSize, 1.0f, drawColor);
            }
        }

        DrawText("@", player.x * tileSize + 4, player.y * tileSize + 2, tileSize, GREEN);
        EndMode2D();

        // HUD / UI
        int hudX = GetScreenWidth() - 250;
        int hudY = 20;
        DrawRectangle(hudX - 10, hudY, 250, 160, BLACK);

        DrawText(TextFormat("%s (%s)", player.name.c_str(), player.className.c_str()), hudX, hudY + 5, 16, GOLD);

        Color modeColor = WHITE; const char* modeSymbol = "";
        switch (currentMode) {
        case MODE_LOOK:  modeColor = SKYBLUE; modeSymbol = "?"; break;
        case MODE_SPEAK: modeColor = GOLD;    modeSymbol = "\""; break;
        case MODE_GRAB:  modeColor = GREEN;   modeSymbol = "m"; break;
        case MODE_STEAL: modeColor = PURPLE;  modeSymbol = "$"; break;
        }

        DrawRectangle(hudX, hudY + 30, 20, 20, modeColor);
        DrawText(modeSymbol, hudX + 5, hudY + 32, 16, BLACK);
        DrawText(TextFormat("MODE: %s", modeNames[currentMode]), hudX + 30, hudY + 32, 16, modeColor);

        DrawRectangle(hudX, hudY + 60, 200, 18, DARKGRAY);
        DrawRectangle(hudX, hudY + 60, (player.hp * 200) / std::max(1, player.maxHp), 18, RED);
        DrawText(TextFormat("HP: %i/%i", player.hp, player.maxHp), hudX + 5, hudY + 61, 14, WHITE);

        DrawRectangle(hudX, hudY + 85, 200, 18, DARKGRAY);
        DrawRectangle(hudX, hudY + 85, (player.stamina * 200) / std::max(1, player.maxStamina), 18, GREEN);
        DrawText(TextFormat("SP: %i/%i", player.stamina, player.maxStamina), hudX + 5, hudY + 86, 14, WHITE);

        DrawRectangle(hudX, hudY + 110, 200, 18, DARKGRAY);
        DrawRectangle(hudX, hudY + 110, (player.mana * 200) / std::max(1, player.maxMana), 18, BLUE);
        DrawText(TextFormat("MP: %i/%i", player.mana, player.maxMana), hudX + 5, hudY + 111, 14, WHITE);

        if (!actionMessage.empty()) {
            int msgWidth = MeasureText(actionMessage.c_str(), 20);
            DrawRectangle((GetScreenWidth() - msgWidth) / 2 - 10, GetScreenHeight() - 45, msgWidth + 20, 30, Fade(BLACK, 0.8f));
            DrawText(actionMessage.c_str(), (GetScreenWidth() - msgWidth) / 2, GetScreenHeight() - 40, 20, RAYWHITE);
        }

        if (showInventory) {
            int invX = GetScreenWidth() / 2 - 250;
            int invY = GetScreenHeight() / 2 - 200;
            DrawRectangle(invX, invY, 500, 400, Fade(BLACK, 0.9f));
            DrawRectangleLines(invX, invY, 500, 400, GOLD);
            DrawText("INVENTORY (Press 'I' to close)", invX + 20, invY + 15, 20, GOLD);
            DrawText("E: Equip | D: Drop | U: Unequip", invX + 20, invY + 370, 16, GRAY);

            DrawText("EQUIPPED:", invX + 270, invY + 60, 18, GOLD);
            int slotDrawY = invY + 90;
            for (int s = 0; s < SLOT_SINGLE_COUNT; s++) {
                if (!player.equippedSlots[s].IsEmpty()) {
                    std::string eqName = GetGroundItemName(player.equippedSlots[s]);
                    DrawText(eqName.c_str(), invX + 270, slotDrawY, 16, WHITE);
                    slotDrawY += 22;
                }
            }

            if (player.inventory.empty()) {
                DrawText("Your inventory is empty.", invX + 20, invY + 60, 18, LIGHTGRAY);
            }
            else {
                for (size_t i = 0; i < player.inventory.size(); i++) {
                    std::string itemName = GetGroundItemName(player.inventory[i]);
                    Color itemColor = WHITE;
                    if ((int)i == selectedItemIndex) {
                        itemColor = YELLOW;
                        DrawText(">", invX + 15, invY + 60 + (int)i * 25, 18, YELLOW);
                    }
                    std::string line = itemName + " x" + std::to_string(player.inventory[i].quantity);
                    DrawText(line.c_str(), invX + 35, invY + 60 + (int)i * 25, 18, itemColor);
                }
            }
        }

        if (showPauseMenu) {
            int menuWidth = 400; int menuHeight = 320;
            int menuX = GetScreenWidth() / 2 - menuWidth / 2;
            int menuY = GetScreenHeight() / 2 - menuHeight / 2;
            DrawRectangle(menuX, menuY, menuWidth, menuHeight, Fade(BLACK, 0.95f));
            DrawRectangleLines(menuX, menuY, menuWidth, menuHeight, GOLD);
            DrawText("PAUSED", menuX + (menuWidth - MeasureText("PAUSED", 24)) / 2, menuY + 20, 24, GOLD);

            const char* pauseOptions[] = { "Resume", "Save Game", "Load Game", "Options", "Return to Title", "Quit Game" };
            Vector2 mousePos = GetMousePosition();
            for (int i = 0; i < 6; i++) {
                int optY = menuY + 75 + i * 35;
                Rectangle optRect = { (float)menuX + 50, (float)optY - 2, (float)menuWidth - 100, 30 };
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
                if (i == pauseMenuSelection) DrawText(">", menuX + 60, optY, 20, YELLOW);
                DrawText(pauseOptions[i], menuX + 85, optY, 20, optColor);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}