#include <raylib.h>
#include <vector>
#include <algorithm> // for std::min, std::max
#include <cmath> // NEW: For calculating stair distance
#include "DungeonData.h" // Links our custom data structures
#include "Player.h" // Links our new Player struct
#include "Enemy.h" // Links our new Enemy struct
#include "Combat.h" // NEW: Bump-combat resolution
#include <vector>  // Ensure we can use lists
#include <string>  // NEW: Ensure we can use strings for text messages

void GenerateFloor(TileType map[mapWidth][mapHeight], bool explored[mapWidth][mapHeight], std::vector<Room>& rooms, Player& player, std::vector<Enemy>& enemies) {
    rooms.clear();
    for (int x = 0; x < mapWidth; x++) {
        for (int y = 0; y < mapHeight; y++) {
            map[x][y] = TILE_WALL;
            explored[x][y] = false;
        }
    }

    int attempts = 0;
    while (rooms.size() < maxRooms && attempts < 100) {
        attempts++;
        int rw = GetRandomValue(4, 8), rh = GetRandomValue(4, 8);
        int rx = GetRandomValue(1, mapWidth - rw - 1), ry = GetRandomValue(1, mapHeight - rh - 1);
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

    // Clear out old enemies when making a new floor
    enemies.clear();

    // Start at 1 to skip the first room (where the player spawns!)
    for (int i = 1; i < rooms.size(); i++) {
        Enemy goblin;
        goblin.x = rooms[i].x + rooms[i].width / 2;
        goblin.y = rooms[i].y + rooms[i].height / 2;
        goblin.str = 30;
        goblin.end = 35;
        goblin.agi = 45;
        goblin.intel = 20;
        goblin.wil = 20;
        goblin.per = 15;
        goblin.lck = 20;

        goblin.maxHp = goblin.end / 5;
        goblin.hp = goblin.maxHp;

        goblin.maxMana = goblin.intel / 5;
        goblin.mana = goblin.maxMana;

        goblin.maxStamina = (goblin.end + goblin.str + goblin.agi + 40) / 10;
        goblin.stamina = goblin.maxStamina;
        goblin.symbol = 'g';
        goblin.color = GREEN;
        goblin.isDead = false;

        enemies.push_back(goblin);
    }
}

int main()
{
    const int tileSize = 24;

    // We accidentally deleted the window setup! Let's put it back.
    // NEW: Tell Raylib we want a resizable window with a maximize button
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Pocalys");
    SetTargetFPS(60);

    // Re-declare the variables so they actually exist in main's memory
    TileType map[mapWidth][mapHeight];
    bool explored[mapWidth][mapHeight] = { false };
    std::vector<Room> rooms;
    std::vector<LevelState> dungeon;
    int currentFloor = 0;
    std::vector<Enemy> enemies; // Holds all monsters on the current floor
    std::string actionMessage = "Welcome to the dungeon!";
    // NEW: Define our four interaction types and a way to display their names
    enum ActionMode { MODE_GRAB, MODE_LOOK, MODE_SPEAK, MODE_STEAL };
    ActionMode currentMode = MODE_LOOK; // Start on Look by default
    const char* modeNames[] = { "GRAB", "LOOK", "SPEAK", "STEAL" };

    // Create the player entity: X, Y, HP, MaxHP, SP, MaxSP, MP, MaxMP
    Player player;
    player.x = 0;
    player.y = 0;

    // Archetype: The Knight
    player.str = 65;
    player.end = 55;
    player.agi = 35;
    player.intel = 25;
    player.wil = 25;
    player.per = 35;
    player.lck = 30;

    player.maxHp = player.end / 5;
    player.hp = player.maxHp;

    player.maxMana = player.intel / 5;
    player.mana = player.maxMana;

    // Base SPD of 40 used for missing speed attribute
    player.maxStamina = (player.end + player.str + player.agi + 40) / 10;
    player.stamina = player.maxStamina;

    // Pass the player struct into our generator
    GenerateFloor(map, explored, rooms, player, enemies);

    // NEW: Save this initial layout into the dungeon memory
    LevelState firstFloor;
    for (int x = 0; x < mapWidth; x++) {
        for (int y = 0; y < mapHeight; y++) {
            firstFloor.savedMap[x][y] = map[x][y];
            firstFloor.savedExplored[x][y] = explored[x][y];
        }
    }
    firstFloor.savedEnemies = enemies;
    dungeon.push_back(firstFloor);

    bool enableFog = true;

    // NEW: Create and configure the 2D Camera
    Camera2D camera = { 0 };
    // Offset the camera so its target is in the exact center of our window
    camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.5f; // Zoom in slightly so the tiles look chunky!
    // NEW: Snap the camera to the player immediately when the game starts
    camera.target = { (float)player.x * tileSize + (tileSize / 2.0f), (float)player.y * tileSize + (tileSize / 2.0f) };

    while (!WindowShouldClose())
    {
        // --- INPUT & UPDATE ---
        // NEW: Constantly update the camera's center in case the window gets resized
        camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        int nextX = player.x;
        int nextY = player.y;

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) nextX++;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) nextX--;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) nextY--;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) nextY++;

        // NEW: Allow the player to walk on floors AND stairs
        // NEW: Allow walking anywhere if fog is off (noclip!), otherwise respect walls
// If the player is trying to move...
        if (nextX != player.x || nextY != player.y) {
            bool enemyBlocking = false;

            // 1. Check if a living enemy is in the way, and attack it if so
            for (auto it = enemies.begin(); it != enemies.end(); ++it) {
                if (it->x == nextX && it->y == nextY && !it->isDead) {
                    enemyBlocking = true;
                    ResolveBumpAttack(player, *it, actionMessage);
                    break;
                }
            }

            // 2. Move if not blocked (Noclip bypasses everything!)
            if (!enableFog || (!enemyBlocking && (map[nextX][nextY] == TILE_FLOOR || map[nextX][nextY] == TILE_STAIR_UP || map[nextX][nextY] == TILE_STAIR_DOWN))) {
                player.x = nextX;
                player.y = nextY;
                if (!enemyBlocking) actionMessage = ""; // Clear message on successful step
            }
        }

        // NEW: The Multi-Interaction Mechanic (Left Click)
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            int clickX = (int)(mouseWorldPos.x / tileSize);
            int clickY = (int)(mouseWorldPos.y / tileSize);

            bool foundSomething = false;

            // Check if we clicked an enemy we can actually see
            for (const auto& enemy : enemies) {
                if (enemy.x == clickX && enemy.y == clickY && (!enableFog || explored[clickX][clickY])) {
                    switch (currentMode) {
                    case MODE_LOOK:
                    {
                        float hpPercent = (float)enemy.hp / enemy.maxHp;
                        if (hpPercent >= 1.0f) actionMessage = "It looks completely unharmed.";
                        else if (hpPercent >= 0.5f) actionMessage = "It has some cuts and bruises.";
                        else if (hpPercent >= 0.2f) actionMessage = "It looks severely wounded!";
                        else actionMessage = "It is clinging to life...";
                    }
                    break;
                    case MODE_SPEAK: actionMessage = "The goblin shrieks at you!"; break;
                    case MODE_GRAB:  actionMessage = "You try to grab it, but it snaps at your fingers!"; break;
                    case MODE_STEAL: actionMessage = "It has nothing worth stealing."; break;
                    }
                    foundSomething = true;
                    break;
                }
            }

            // Easter egg for clicking yourself
            if (!foundSomething && clickX == player.x && clickY == player.y) {
                switch (currentMode) {
                case MODE_LOOK:  actionMessage = "That's you. You're doing great."; break;
                case MODE_SPEAK: actionMessage = "Talking to yourself again?"; break;
                case MODE_GRAB:  actionMessage = "You give yourself a comforting hug."; break;
                case MODE_STEAL: actionMessage = "What have I got in my pocket?"; break;
                }
                foundSomething = true;
            }

            // Clicking empty space
            if (!foundSomething) {
                switch (currentMode) {
                case MODE_LOOK:  actionMessage = "You see nothing of interest."; break;
                case MODE_SPEAK: actionMessage = "Hello, empty room."; break;
                case MODE_GRAB:  actionMessage = "You grasp at empty air."; break;
                case MODE_STEAL: actionMessage = "There's nothing to steal here."; break;
                }
            }
        }

        // NEW: Press 'F' to toggle the Fog of War for testing
        if (IsKeyPressed(KEY_F)) enableFog = !enableFog;

        // NEW: Toggle Fullscreen with F11
        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

        // NEW: Cycle interaction modes with the Mouse Wheel
        float wheelMove = GetMouseWheelMove();
        if (wheelMove > 0) {
            currentMode = (ActionMode)((currentMode + 1) % 4); // Scroll Up
        }
        else if (wheelMove < 0) {
            currentMode = (ActionMode)((currentMode + 3) % 4); // Scroll Down (+3 safely loops backward in a 4-item list!)
        }

        // NEW: Update the camera to follow the player's pixel position
        // 1. Figure out exactly where the camera SHOULD be
        float targetX = (float)player.x * tileSize + (tileSize / 2.0f);
        float targetY = (float)player.y * tileSize + (tileSize / 2.0f);

        // 2. Glide the camera 10% of the way toward that target every second (Lerping!)
        camera.target.x += (targetX - camera.target.x) * 10.0f * GetFrameTime();
        camera.target.y += (targetY - camera.target.y) * 10.0f * GetFrameTime();

        // NEW: Ascending and Descending logic
        if (IsKeyPressed(KEY_SPACE)) {
            bool wentDown = (map[player.x][player.y] == TILE_STAIR_DOWN);
            bool wentUp = (map[player.x][player.y] == TILE_STAIR_UP && currentFloor > 0);

            if (wentDown || wentUp) {
                // 1. Save the current floor before we leave it
                for (int x = 0; x < mapWidth; x++) {
                    for (int y = 0; y < mapHeight; y++) {
                        dungeon[currentFloor].savedMap[x][y] = map[x][y];
                        dungeon[currentFloor].savedExplored[x][y] = explored[x][y];
                    }
                }
                dungeon[currentFloor].savedEnemies = enemies;

                // 2. Change Floor
                if (wentDown) currentFloor++;
                if (wentUp) currentFloor--;

                // 3. Are we generating a NEW floor, or loading an OLD one?
                if (currentFloor >= dungeon.size()) {
                    // --- GENERATE NEW FLOOR ---
                    GenerateFloor(map, explored, rooms, player, enemies);

                    // Push empty state to reserve space in memory
                    LevelState newFloor;
                    dungeon.push_back(newFloor);
                }
                else {
                    // --- LOAD EXISTING FLOOR ---
                    for (int x = 0; x < mapWidth; x++) {
                        for (int y = 0; y < mapHeight; y++) {
                            map[x][y] = dungeon[currentFloor].savedMap[x][y];
                            explored[x][y] = dungeon[currentFloor].savedExplored[x][y];
                        }
                    }
                    enemies = dungeon[currentFloor].savedEnemies;

                    // Find correct stair to spawn on
                    TileType targetStair = wentDown ? TILE_STAIR_UP : TILE_STAIR_DOWN;
                    for (int x = 0; x < mapWidth; x++) {
                        for (int y = 0; y < mapHeight; y++) {
                            if (map[x][y] == targetStair) {
                                player.x = x + 1;
                                player.y = y;
                            }
                        }
                    }
                }
                continue; // Skip the draw call this frame to prevent glitches
            }
        }

        // NEW: Reveal a 5x5 grid around the player's current position
        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                int viewX = player.x + i;
                int viewY = player.y + j;

                // Make sure we don't try to reveal tiles outside the array bounds
                if (viewX >= 0 && viewX < mapWidth && viewY >= 0 && viewY < mapHeight) {
                    explored[viewX][viewY] = true;
                }
            }
        }

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);
        // NEW: Activate the camera view
        BeginMode2D(camera);

        // Expand the drawing bounds by 30 tiles in every direction to hide the void!
        for (int x = -30; x < mapWidth + 30; x++) {
            for (int y = -30; y < mapHeight + 30; y++) {

                // If the tile is outside our real map array, draw a fake wall and skip the rest!
                if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
                    // 1. Find the closest actual border tile on our real map
                    int edgeX = std::max(0, std::min(x, mapWidth - 1));
                    int edgeY = std::max(0, std::min(y, mapHeight - 1));

                    // 2. If the border is explored, we pretend this fake wall is explored too
                    // (But only up to 2 tiles deep, to perfectly mimic our 5x5 vision radius!)
                    bool fakeExplored = explored[edgeX][edgeY] && (std::abs(x - edgeX) <= 2) && (std::abs(y - edgeY) <= 2);

                    // 3. Draw it only if it passes the fog check
                    if (fakeExplored || !enableFog) {
                        DrawText("#", x * tileSize + 4, y * tileSize + 2, tileSize, DARKGRAY);
                    }
                    continue;
                }
                // NEW: Only draw the tile if the player has explored it
                // NEW: Draw if explored, OR if the fog is toggled off
                if (explored[x][y] || !enableFog) {
                    if (map[x][y] == TILE_WALL) {
                        DrawText("#", x * tileSize + 4, y * tileSize + 2, tileSize, DARKGRAY);
                    }
                    else if (map[x][y] == TILE_FLOOR) {
                        DrawText(".", x * tileSize + 8, y * tileSize + 2, tileSize, ColorAlpha(DARKGRAY, 0.5f));
                    }
                    else if (map[x][y] == TILE_STAIR_UP) {
                        DrawText("<", x * tileSize + 4, y * tileSize + 2, tileSize, YELLOW);
                    }
                    else if (map[x][y] == TILE_STAIR_DOWN) {
                        DrawText(">", x * tileSize + 4, y * tileSize + 2, tileSize, YELLOW);
                    }
                }
            }
        }

        // 0. Draw the enemies (only if visible or fog is disabled)
        for (const auto& enemy : enemies) {
            if (!enableFog || explored[enemy.x][enemy.y]) {
                const char* enemyChar = TextFormat("%c", enemy.symbol);

                Color drawColor = enemy.color;
                float rotation = 0.0f;
                if (enemy.isDead) {
                    drawColor = GRAY;
                    rotation = 90.0f;
                }

                Vector2 textSize = MeasureTextEx(GetFontDefault(), enemyChar, (float)tileSize, 1.0f);
                Vector2 origin = { textSize.x / 2.0f, textSize.y / 2.0f };
                Vector2 position = { enemy.x * tileSize + tileSize / 2.0f, enemy.y * tileSize + tileSize / 2.0f };

                DrawTextPro(GetFontDefault(), enemyChar, position, origin, rotation, (float)tileSize, 1.0f, drawColor);
            }
        }

        // 1. Draw the player in the world
        DrawText("@", player.x* tileSize + 4, player.y* tileSize + 2, tileSize, GREEN);

        // 2. Turn OFF the camera view (everything after this is glued to the screen!)
        EndMode2D();

        // 3. Draw the HUD
// --- HUD / UI ---
        int hudX = GetScreenWidth() - 220;
        int hudY = 20;

        // Made the background taller (140 instead of 110) to fit the icon
        DrawRectangle(hudX - 10, hudY, 220, 140, BLACK);

        // NEW: Figure out what color and symbol to draw based on the current mode
        Color modeColor = WHITE;
        const char* modeSymbol = "";
        switch (currentMode) {
        case MODE_LOOK:  modeColor = SKYBLUE; modeSymbol = "?"; break;
        case MODE_SPEAK: modeColor = GOLD;    modeSymbol = "\""; break;
        case MODE_GRAB:  modeColor = GREEN;   modeSymbol = "m"; break; // Looks like a hand
        case MODE_STEAL: modeColor = PURPLE;  modeSymbol = "$"; break;
        }

        // NEW: Draw the Icon Box and the Mode Text
        DrawRectangle(hudX, hudY + 10, 24, 24, modeColor);
        DrawText(modeSymbol, hudX + 7, hudY + 14, 20, BLACK);
        DrawText(TextFormat("MODE: %s", modeNames[currentMode]), hudX + 35, hudY + 14, 18, modeColor);

        // Health Bar (Red) - Shifted down to +40
        DrawRectangle(hudX, hudY + 40, 200, 20, DARKGRAY);
        DrawRectangle(hudX, hudY + 40, (player.hp * 200) / player.maxHp, 20, RED);
        DrawText(TextFormat("HP: %i/%i", player.hp, player.maxHp), hudX + 5, hudY + 42, 16, WHITE);

        // Stamina Bar (Green) - Shifted down to +70
        DrawRectangle(hudX, hudY + 70, 200, 20, DARKGRAY);
        DrawRectangle(hudX, hudY + 70, (player.stamina * 200) / player.maxStamina, 20, GREEN);
        DrawText(TextFormat("SP: %i/%i", player.stamina, player.maxStamina), hudX + 5, hudY + 72, 16, WHITE);

        // Mana Bar (Blue) - Shifted down to +100
        DrawRectangle(hudX, hudY + 100, 200, 20, DARKGRAY);
        DrawRectangle(hudX, hudY + 100, (player.mana * 200) / player.maxMana, 20, BLUE);
        DrawText(TextFormat("MP: %i/%i", player.mana, player.maxMana), hudX + 5, hudY + 102, 16, WHITE);

        // Draw the Action Message at the bottom center of the screen
        if (actionMessage != "") {
            int msgWidth = MeasureText(actionMessage.c_str(), 20);
            DrawRectangle(GetScreenWidth() / 2 - msgWidth / 2 - 10, GetScreenHeight() - 50, msgWidth + 20, 30, Fade(BLACK, 0.8f));
            DrawText(actionMessage.c_str(), GetScreenWidth() / 2 - msgWidth / 2, GetScreenHeight() - 45, 20, WHITE);
        }

        // 4. Finish the frame
        EndDrawing();
        // NEW: Deactivate the camera view before drawing the UI
        EndMode2D();

    }

    CloseWindow();
    return 0;
}