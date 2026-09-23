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
#include <string>

const int MAP_WIDTH = 80;
const int MAP_HEIGHT = 45;
const int MAX_ROOMS = 15;

enum GameState {
    STATE_TITLE,
    STATE_TAVERN,
    STATE_GAMEPLAY
};

enum TitleTheme {
    THEME_FIRE,
    THEME_RAIN,
    THEME_ICE,
    THEME_ELECTRIC,
    THEME_COUNT
};

const std::vector<std::string> G_TITLE_BANNER = {
    " ____    ____    ____     _     _    __   __ _____  ",
    "|  _ \\  / __ \\  / ___|   / \\   | |   \\ \\ / // ___| ",
    "| |_) || |  | || |      / _ \\  | |    \\ V / \\___ \\ ",
    "|  __/ | |__| || |___  / ___ \\ | |___  | |   ___) |",
    "|_|     \\____/  \\____|/_/   \\_\\|_____| |_|  |____/  "
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

// --- FIRE ANIMATION ---
Color GetFireColor(int intensity) {
    if (intensity <= 0) return BLACK;
    if (intensity < 7)  return Color{ 70, 10, 15, 255 };
    if (intensity < 14) return Color{ 170, 30, 10, 255 };
    if (intensity < 21) return Color{ 230, 90, 10, 255 };
    if (intensity < 28) return Color{ 245, 190, 20, 255 };
    return Color{ 255, 240, 200, 255 };
}

char GetFireChar(int intensity) {
    const char fireRamp[] = " .:-=+*#%@";
    int idx = std::min(9, std::max(0, (intensity * 10) / 36));
    return fireRamp[idx];
}

void UpdateFire(std::vector<int>& fireGrid, int width, int height) {
    for (int x = 0; x < width; x++) {
        fireGrid[(height - 1) * width + x] = (GetRandomValue(0, 3) == 0) ? 0 : 35;
    }
    for (int y = 1; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcIndex = y * width + x;
            int srcValue = fireGrid[srcIndex];
            if (srcValue == 0) {
                fireGrid[(y - 1) * width + x] = 0;
            }
            else {
                int randX = GetRandomValue(-1, 1);
                int decay = GetRandomValue(0, 2);
                int dstX = std::max(0, std::min(width - 1, x + randX));
                int dstY = y - 1;
                fireGrid[dstY * width + dstX] = std::max(0, srcValue - decay);
            }
        }
    }
}

// Directionally weighted edge detection (Outward gradient from banner center)
// Restricts lightning source pixels to the upper portion of the banner
bool IsValidLightningSource(int x, int y, int startY, int bannerH)
{
    int relativeY = y - startY;
    if (relativeY <= 1)
    {
        return true;
    }
    else
    {
        return false;
    }
    //int relativeY = y - startY;
    //return relativeY <= (int)(bannerH * 0.6f); // Only fire from top 60% of banner
}

// Maps source position relative to banner center out to top/side screen edges
Vector2 GetRadialScreenEdge(Vector2 point, Vector2 bannerCenter, int gridW, int gridH) {
    float dx = point.x - bannerCenter.x;
    float dy = point.y - bannerCenter.y;

    // Angle in radians relative to center (-PI to PI, where -PI/2 is straight up)
    float angle = atan2f(dy, dx);

    // Left Screen Edge (Angle roughly between -180° and -135°)
    if (angle < -0.75f * PI) {
        float normY = (angle + PI) / (0.25f * PI); // 0.0 to 1.0 down left edge
        return { 0.0f, std::min((float)gridH - 1.0f, (gridH * 0.4f) * normY) };
    }
    // Right Screen Edge (Angle roughly between -45° and 0°)
    else if (angle > -0.25f * PI) {
        float normY = std::abs(angle) / (0.25f * PI); // 0.0 to 1.0 down right edge
        return { (float)(gridW - 1), std::min((float)gridH - 1.0f, (gridH * 0.4f) * normY) };
    }
    // Top Edge Arc (-135° to -45°)
    else {
        // Interpolate across top boundary from far-left to far-right
        float t = (angle + 0.75f * PI) / (0.50f * PI); // 0.0 (left side) to 1.0 (right side)
        return { t * (gridW - 1), 0.0f };
    }
}

// --- ASCII LIGHTNING RENDERER ---
void DrawASCIILine(Vector2 startGrid, Vector2 endGrid, float cellW, float cellH, Color color, int maxJitter = 1) {
    int x0 = (int)startGrid.x;
    int y0 = (int)startGrid.y;
    int x1 = (int)endGrid.x;
    int y1 = (int)endGrid.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int currX = x0;
    int currY = y0;

    while (true) {
        int jitterX = (maxJitter > 0) ? GetRandomValue(-maxJitter, maxJitter) : 0;
        int jitterY = (maxJitter > 0) ? GetRandomValue(-maxJitter, maxJitter) : 0;

        int drawX = currX + jitterX;
        int drawY = currY + jitterY;

        char charToDraw = '*';
        if (dx > dy * 2) charToDraw = '-';
        else if (dy > dx * 2) charToDraw = '|';
        else if ((sx > 0 && sy > 0) || (sx < 0 && sy < 0)) charToDraw = '\\';
        else charToDraw = '/';

        if (GetRandomValue(0, 5) == 0) charToDraw = '+';

        char str[2] = { charToDraw, '\0' };
        DrawText(str, (int)(drawX * cellW), (int)(drawY * cellH), (int)cellH, color);

        if (currX == x1 && currY == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; currX += sx; }
        if (e2 < dx) { err += dx; currY += sy; }
    }
}

void DrawSustainedASCIILightning(Vector2 startGrid, Vector2 endGrid, float cellW, float cellH, Color mainCol, Color subCol, int gridW, int gridH) {
    Vector2 midGrid = {
        (startGrid.x + endGrid.x) * 0.5f + GetRandomValue(-2, 2),
        (startGrid.y + endGrid.y) * 0.5f + GetRandomValue(-2, 2)
    };

    DrawASCIILine(startGrid, midGrid, cellW, cellH, mainCol, 0);
    DrawASCIILine(midGrid, endGrid, cellW, cellH, mainCol, 0);

    if (GetRandomValue(0, 1) == 0) {
        Vector2 subArcTarget = {
            endGrid.x + GetRandomValue(-6, 6),
            endGrid.y + GetRandomValue(-6, 6)
        };
        subArcTarget.x = std::max(0.0f, std::min((float)(gridW - 1), subArcTarget.x));
        subArcTarget.y = std::max(0.0f, std::min((float)(gridH - 1), subArcTarget.y));

        DrawASCIILine(endGrid, subArcTarget, cellW, cellH, subCol, 1);
    }
}

std::string GetEnemyName(const Enemy& enemy) {
    if (enemy.symbol == 'g') return "goblin";
    return "creature";
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

void GenerateFloor(TileType map[MAP_WIDTH][MAP_HEIGHT], bool explored[MAP_WIDTH][MAP_HEIGHT], std::vector<Room>& rooms, Player& player, std::vector<Enemy>& enemies) {
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
        Enemy goblin;
        goblin.x = rooms[i].x + rooms[i].width / 2;
        goblin.y = rooms[i].y + rooms[i].height / 2;
        goblin.str = 30; goblin.end = 35; goblin.agi = 45;
        goblin.intel = 20; goblin.wil = 20; goblin.per = 15; goblin.lck = 20;
        goblin.maxHp = goblin.end / 5; goblin.hp = goblin.maxHp;
        goblin.maxMana = goblin.intel / 5; goblin.mana = goblin.maxMana;
        goblin.maxStamina = (goblin.end + goblin.str + goblin.agi + 40) / 10; goblin.stamina = goblin.maxStamina;
        goblin.symbol = 'g'; goblin.color = GREEN; goblin.isDead = false;
        enemies.push_back(goblin);
    }
}

void ApplyProfileToPlayer(const CharacterProfile& profile, Player& player) {
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
}


struct Snowflake {
    float x, y;
    float speed;
    char symbol;
    Color color;
};

// Cascades snow along banner top edges and slides excess snow off cliff edges
void UpdateSnowPhysics(
    std::vector<int>& snowGrid,
    std::vector<Snowflake>& snowflakes,
    int gridW,
    int gridH,
    int startX,
    int startY,
    int bannerW,
    int bannerH,
    const std::vector<std::string>& banner
) {
    auto IsPixel = [&](int x, int y) {
        int bx = x - startX, by = y - startY;
        return (by >= 0 && by < bannerH && bx >= 0 && bx < bannerW && banner[by][bx] != ' ');
        };

    auto IsTopEdge = [&](int x, int y) {
        if (!IsPixel(x, y)) return false;
        return !IsPixel(x, y - 1);
        };

    for (int y = 0; y < gridH; y++) {
        for (int x = 0; x < gridW; x++) {
            int idx = y * gridW + x;
            if (snowGrid[idx] <= 0) continue;

            // Clear any accumulation that isn't on a top edge tile
            if (!IsTopEdge(x, y)) {
                snowGrid[idx] = 0;
                continue;
            }

            int dir = (GetRandomValue(0, 1) == 0) ? -1 : 1;
            for (int i = 0; i < 2; i++, dir = -dir) {
                int nx = x + dir;
                if (nx < 0 || nx >= gridW) continue;

                int ny = -1;
                if (IsTopEdge(nx, y)) ny = y;
                else if (IsTopEdge(nx, y + 1)) ny = y + 1;
                else if (IsTopEdge(nx, y - 1)) ny = y - 1;

                if (ny != -1) {
                    // Balance mound height between adjacent top edges
                    int nIdx = ny * gridW + nx;
                    if (snowGrid[idx] > snowGrid[nIdx] + 1) {
                        snowGrid[idx]--;
                        snowGrid[nIdx]++;
                        break;
                    }
                }
                else {
                    // Edge of letter / cliff: slide snow off into free fall
                    if (snowGrid[idx] > 2 && GetRandomValue(0, 3) == 0) {
                        snowGrid[idx]--;
                        for (auto& flake : snowflakes) {
                            if (flake.y <= 0 || flake.y >= gridH - 1) {
                                flake.x = (float)nx;
                                flake.y = (float)y;
                                flake.speed = (float)GetRandomValue(3, 7) / 10.0f;
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}

// Cascades snow across adjacent pixels when height difference > 1
void UpdateSnowPhysics(std::vector<int>& snowGrid, int gridW, int gridH) {
    for (int y = 0; y < gridH; y++) {
        for (int x = 0; x < gridW; x++) {
            int idx = y * gridW + x;
            if (snowGrid[idx] <= 1) continue;

            // Check if left neighbor has space
            if (x > 0 && snowGrid[idx] > snowGrid[y * gridW + (x - 1)] + 1) {
                snowGrid[idx]--;
                snowGrid[y * gridW + (x - 1)]++;
            }
            // Check if right neighbor has space
            else if (x < gridW - 1 && snowGrid[idx] > snowGrid[y * gridW + (x + 1)] + 1) {
                snowGrid[idx]--;
                snowGrid[y * gridW + (x + 1)]++;
            }
        }
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
    int titleMenuSelection = 0;

    TitleTheme currentTitleTheme = (TitleTheme)GetRandomValue(0, THEME_COUNT - 1);

    std::vector<CharacterProfile> tavernCandidates;
    int selectedCandidate = 0;

    const int GRID_W = 90; const int GRID_H = 45;
    std::vector<int> fireGrid(GRID_W * GRID_H, 0);

    // Raindrops
    struct RainDrop {
        float x, y;
        float speed;
        int impactTimer;
    };
    std::vector<RainDrop> rainDrops(60);
    for (auto& drop : rainDrops) {
        drop.x = (float)GetRandomValue(0, GRID_W - 1);
        drop.y = (float)GetRandomValue(0, GRID_H - 1);
        drop.speed = (float)GetRandomValue(12, 20) / 10.0f;
        drop.impactTimer = 0;
    }

    // Snowflakes (struct definition moved above UpdateSnowPhysics)
    std::vector<Snowflake> snowflakes(50);
    const char flakeChars[] = "*+.o#";
    for (auto& flake : snowflakes) {
        flake.x = (float)GetRandomValue(0, GRID_W - 1);
        flake.y = (float)GetRandomValue(0, GRID_H - 1);
        flake.speed = (float)GetRandomValue(2, 6) / 10.0f;
        flake.symbol = flakeChars[GetRandomValue(0, 4)];
        flake.color = (GetRandomValue(0, 1) == 0) ? SKYBLUE : RAYWHITE;
    }
    std::vector<int> snowAccumulation(GRID_W * GRID_H, 0);

    // Electric Arc Particles
    struct ElectricParticle {
        float x, y;
        float vx, vy;
        float alpha;
        char symbol;
    };
    std::vector<ElectricParticle> arcParticles(35);
    const char sparkChars[] = "*+~:-";
    for (auto& p : arcParticles) {
        p.x = (float)GetRandomValue(0, GRID_W - 1);
        p.y = (float)GetRandomValue(0, GRID_H - 1);
        p.vx = (float)GetRandomValue(-8, 8) / 10.0f;
        p.vy = (float)GetRandomValue(-8, 8) / 10.0f;
        p.alpha = (float)GetRandomValue(30, 100) / 100.0f;
        p.symbol = sparkChars[GetRandomValue(0, 4)];
    }

    float electricCycleTimer = 0.0f;
    struct LightningAnchor {
        Vector2 start;
        Vector2 end;
    };
    std::vector<LightningAnchor> persistentAnchors;

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
        float frameDt = GetFrameTime();
        UpdateFire(fireGrid, GRID_W, GRID_H);

        // --- TITLE STATE ---
        if (currentState == STATE_TITLE)
        {
            if (IsKeyPressed(KEY_T)) {
                currentTitleTheme = (TitleTheme)((currentTitleTheme + 1) % THEME_COUNT);
                electricCycleTimer = 0.0f;
                persistentAnchors.clear();
                std::fill(snowAccumulation.begin(), snowAccumulation.end(), 0);
            }

            const int titleOptionCount = 3;
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                titleMenuSelection--;
                if (titleMenuSelection < 0) titleMenuSelection = titleOptionCount - 1;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                titleMenuSelection++;
                if (titleMenuSelection >= titleOptionCount) titleMenuSelection = 0;
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_E)) {
                if (titleMenuSelection == 0) {
                    tavernCandidates = GenerateTavernCandidates(6);
                    selectedCandidate = 0;
                    currentState = STATE_TAVERN;
                }
                else if (titleMenuSelection == 1) {
                    actionMessage = "Options coming soon!";
                }
                else if (titleMenuSelection == 2) {
                    keepRunning = false;
                }
            }

            BeginDrawing();
            ClearBackground(BLACK);
            float cellW = (float)GetScreenWidth() / GRID_W;
            float cellH = (float)GetScreenHeight() / GRID_H;
            int bannerW = (int)G_TITLE_BANNER[0].length();
            int bannerH = (int)G_TITLE_BANNER.size();
            int startX = (GRID_W - bannerW) / 2;
            int startY = 10;

            Vector2 bannerCenter = { startX + bannerW * 0.5f, startY + bannerH * 0.5f };

            auto IsBannerPixel = [&](int x, int y) {
                int bx = x - startX, by = y - startY;
                if (by < 0 || by >= bannerH) return false;
                if (bx < 0 || bx >= (int)G_TITLE_BANNER[by].length()) return false; // Check against row's real length
                return G_TITLE_BANNER[by][bx] != ' ';
                };

            auto IsBannerRoofPixel = [&](int x, int y) {
                if (!IsBannerPixel(x, y))
                {
                    return false;
                }
                for (int checkY = 0; checkY < y; checkY++)
                {
                    if (IsBannerPixel(x, checkY))
                    {
                        return false;
                    }
                }
                return true;
                };

            std::vector<Vector2> bannerPixels;
            if (currentTitleTheme == THEME_ELECTRIC) {
                for (int y = 0; y < GRID_H; y++) {
                    for (int x = 0; x < GRID_W; x++) {
                        if (IsBannerPixel(x, y) && IsValidLightningSource(x, y, startY, bannerH)) {
                            bannerPixels.push_back({ (float)x, (float)y });
                        }
                    }
                }
            }

            // RENDER SELECTED ANIMATION THEME
            if (currentTitleTheme == THEME_FIRE) {
                for (int y = 0; y < GRID_H; y++) {
                    for (int x = 0; x < GRID_W; x++) {
                        int intensity = fireGrid[y * GRID_W + x];
                        if (IsBannerPixel(x, y)) {
                            int bx = x - startX, by = y - startY;
                            char c = G_TITLE_BANNER[by][bx]; char str[2] = { c, '\0' };
                            Color dimmedFire = GetFireColor(intensity / 3.5f);
                            DrawText(str, (int)(x * cellW), (int)(y * cellH), (int)cellH, (intensity > 0) ? dimmedFire : Color{ 55, 35, 40, 255 });
                        }
                        else if (intensity > 0) {
                            Color fireColor = GetFireColor(intensity);
                            char c = GetFireChar(intensity); char str[2] = { c, '\0' };
                            DrawText(str, (int)(x * cellW), (int)(y * cellH), (int)cellH, fireColor);
                        }
                    }
                }
            }
            else if (currentTitleTheme == THEME_RAIN) {
                for (auto& drop : rainDrops) {
                    if (drop.impactTimer > 0) {
                        drop.impactTimer--;
                        if (drop.impactTimer <= 0) {
                            drop.y = 0;
                            drop.x = (float)GetRandomValue(0, GRID_W - 1);
                        }
                        continue;
                    }

                    drop.y += drop.speed;
                    drop.x += 0.35f;
                    if (drop.x >= GRID_W)
                    {
                        drop.x -= GRID_W;
                    }
                    if (drop.x < 0)
                    {
                        drop.x += GRID_W;
                    }
                    int currX = (int)drop.x;
                    int currY = (int)drop.y;

                    if (IsBannerRoofPixel(currX, currY))
                    {
                        drop.y = (float)currY;
                        drop.impactTimer = 4;
                    }
                    else if (drop.y >= GRID_H)
                    {
                        drop.y = 0;
                        drop.x = (float)GetRandomValue(0, GRID_W - 1);
                    }
                }

                for (const auto& drop : rainDrops) {
                    int dx = (int)drop.x;
                    int dy = (int)drop.y;
                    if (drop.impactTimer > 0) {
                        DrawText("o", (int)(dx * cellW), (int)(dy * cellH - cellH * 0.5f), (int)cellH, WHITE);
                    }
                    else if (!IsBannerPixel(dx, dy))
                    {
                        DrawText("/", (int)(dx * cellW), (int)(dy * cellH), (int)cellH, Color{ 100, 160, 240, 180 });
                    }
                }

                for (int y = 0; y < GRID_H; y++) {
                    for (int x = 0; x < GRID_W; x++) {
                        if (IsBannerPixel(x, y)) {
                            int bx = x - startX, by = y - startY;
                            char c = G_TITLE_BANNER[by][bx]; char str[2] = { c, '\0' };
                            DrawText(str, (int)(x * cellW), (int)(y * cellH), (int)cellH, SKYBLUE);
                        }
                    }
                }
            }
            else if (currentTitleTheme == THEME_ICE) {
                UpdateSnowPhysics(snowAccumulation, snowflakes, GRID_W, GRID_H, startX, startY, bannerW, bannerH, G_TITLE_BANNER);

                for (auto& flake : snowflakes) {
                    flake.y += flake.speed;
                    int fx = (int)flake.x;
                    int fy = (int)flake.y;

                    if (IsBannerRoofPixel(fx, fy))
                    {
                        int gridIdx = fy * GRID_W + fx;
                        if (gridIdx >= 0 && gridIdx < GRID_W * GRID_H) {
                            if (snowAccumulation[gridIdx] < 6) {
                                snowAccumulation[gridIdx]++;
                                flake.y = 0;
                                flake.x = (float)GetRandomValue(0, GRID_W - 1);
                            }
                        }
                    }
                    else if (flake.y >= GRID_H) {
                        flake.y = 0;
                        flake.x = (float)GetRandomValue(0, GRID_W - 1);
                    }
                }

                for (const auto& flake : snowflakes) {
                    int fx = (int)flake.x;
                    int fy = (int)flake.y;
                    if (!IsBannerPixel(fx, fy)) {
                        char str[2] = { flake.symbol, '\0' };
                        DrawText(str, (int)(fx * cellW), (int)(fy * cellH), (int)cellH, flake.color);
                    }
                }

                for (int y = 0; y < GRID_H; y++) {
                    for (int x = 0; x < GRID_W; x++) {
                        int gridIdx = y * GRID_W + x;
                        if (IsBannerPixel(x, y)) {
                            int bx = x - startX, by = y - startY;
                            char c = G_TITLE_BANNER[by][bx]; char str[2] = { c, '\0' };
                            DrawText(str, (int)(x * cellW), (int)(y * cellH), (int)cellH, Color{ 200, 230, 255, 255 });
                        }
                        if (snowAccumulation[gridIdx] > 0) {
                            const char snowChars[] = "._-:=+*#";
                            int stackLevel = std::min(7, snowAccumulation[gridIdx] - 1);
                            char str[2] = { snowChars[stackLevel], '\0' };
                            DrawText(str, (int)(x * cellW), (int)((y - 0.4f) * cellH), (int)cellH, RAYWHITE);
                        }
                    }
                }
            }
            else if (currentTitleTheme == THEME_ELECTRIC) {
                float prevTimer = electricCycleTimer;
                electricCycleTimer += frameDt;
                if (electricCycleTimer > 5.0f) electricCycleTimer -= 5.0f;

                bool isCharging = (electricCycleTimer >= 3.2f && electricCycleTimer < 4.2f);
                bool isBursting = (electricCycleTimer >= 4.2f && electricCycleTimer < 4.8f);

                if (prevTimer < 4.2f && isBursting && !bannerPixels.empty()) {
                    persistentAnchors.clear();
                    for (int a = 0; a < 5; a++) {
                        int idxA = GetRandomValue(0, (int)bannerPixels.size() - 1);
                        Vector2 pA = bannerPixels[idxA];
                        Vector2 pB = GetRadialScreenEdge(pA, bannerCenter, GRID_W, GRID_H);

                        persistentAnchors.push_back({ pA, pB });
                    }
                }
                else if (!isBursting) {
                    persistentAnchors.clear();
                }

                for (auto& p : arcParticles) {
                    p.x += p.vx; p.y += p.vy;
                    if (p.x < 0 || p.x >= GRID_W) p.vx *= -1;
                    if (p.y < 0 || p.y >= GRID_H) p.vy *= -1;
                    int px = (int)p.x; int py = (int)p.y;
                    if (!IsBannerPixel(px, py)) {
                        char str[2] = { p.symbol, '\0' };
                        Color pCol = (GetRandomValue(0, 1) == 0) ? Color{ 120, 60, 220, (unsigned char)(p.alpha * 150) } : Color{ 60, 180, 240, (unsigned char)(p.alpha * 150) };
                        DrawText(str, (int)(px * cellW), (int)(py * cellH), (int)cellH, pCol);
                    }
                }

                for (int y = 0; y < GRID_H; y++) {
                    for (int x = 0; x < GRID_W; x++) {
                        if (IsBannerPixel(x, y)) {
                            int bx = x - startX, by = y - startY;
                            char c = G_TITLE_BANNER[by][bx]; char str[2] = { c, '\0' };

                            Color bannerCol = Color{ 80, 180, 255, 255 };
                            if (isCharging) {
                                bannerCol = (GetRandomValue(0, 1) == 0) ? Color{ 160, 230, 255, 255 } : WHITE;
                            }
                            else if (isBursting) {
                                bannerCol = (GetRandomValue(0, 3) == 0) ? RAYWHITE : SKYBLUE;
                            }

                            DrawText(str, (int)(x * cellW), (int)(y * cellH), (int)cellH, bannerCol);
                        }
                    }
                }

                if (isBursting && !persistentAnchors.empty()) {
                    for (const auto& anchor : persistentAnchors) {
                        DrawSustainedASCIILightning(anchor.start, anchor.end, cellW, cellH, RAYWHITE, PURPLE, GRID_W, GRID_H);
                    }
                }
                else if (!bannerPixels.empty()) {
                    int idleBolts = isCharging ? 2 : 1;
                    for (int i = 0; i < idleBolts; i++) {
                        int idxA = GetRandomValue(0, (int)bannerPixels.size() - 1);
                        int idxB = GetRandomValue(0, (int)bannerPixels.size() - 1);
                        Color mainCol = isCharging ? WHITE : SKYBLUE;
                        DrawSustainedASCIILightning(bannerPixels[idxA], bannerPixels[idxB], cellW, cellH, mainCol, Color{ 60, 120, 240, 200 }, GRID_W, GRID_H);
                    }
                }
            }

            int menuX = GetScreenWidth() / 2 - 100;
            int menuY = GetScreenHeight() / 2 + 80;
            const char* options[] = { "New Game", "Options", "Quit Game" };
            Vector2 mousePos = GetMousePosition();

            for (int i = 0; i < titleOptionCount; i++) {
                int optY = menuY + i * 40;
                Rectangle optRect = { (float)menuX - 20, (float)optY, 240, 35 };

                if (CheckCollisionPointRec(mousePos, optRect)) {
                    titleMenuSelection = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (i == 0) {
                            tavernCandidates = GenerateTavernCandidates(6);
                            selectedCandidate = 0;
                            currentState = STATE_TAVERN;
                        }
                        else if (i == 1) actionMessage = "Options coming soon!";
                        else if (i == 2) keepRunning = false;
                    }
                }

                Color col = (i == titleMenuSelection) ? YELLOW : WHITE;
                if (i == titleMenuSelection) DrawText(">", menuX - 20, optY, 24, YELLOW);
                DrawText(options[i], menuX, optY, 24, col);
            }

            EndDrawing();
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
                DrawText(TextFormat("Gear: %s", mainHand.c_str()), cx + 12, cy + 118, 13, SKYBLUE);
            }

            std::string hint = "[WASD / Arrows / 1-6] Select  |  [ENTER / Click] Begin Quest  |  [R] Reroll Patrons";
            DrawText(hint.c_str(), GetScreenWidth() / 2 - MeasureText(hint.c_str(), 18) / 2, GetScreenHeight() - 50, 18, GOLD);

            EndDrawing();

            if (confirmSelection) {
                ApplyProfileToPlayer(tavernCandidates[selectedCandidate], player);

                dungeon.clear();
                currentFloor = 0;
                groundItems.clear();
                GenerateFloor(map, explored, rooms, player, enemies);

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
                    GenerateFloor(map, explored, rooms, player, enemies);
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

        for (int x = -30; x < MAP_WIDTH + 30; x++) {
            for (int y = -30; y < MAP_HEIGHT + 30; y++) {
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
                            currentTitleTheme = (TitleTheme)GetRandomValue(0, THEME_COUNT - 1);
                            electricCycleTimer = 0.0f;
                            persistentAnchors.clear();
                            std::fill(snowAccumulation.begin(), snowAccumulation.end(), 0);
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