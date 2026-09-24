#include "TitleScreen.h"
#include <raylib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>

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

    // Left Screen Edge (Angle roughly between -180deg and -135deg)
    if (angle < -0.75f * PI) {
        float normY = (angle + PI) / (0.25f * PI); // 0.0 to 1.0 down left edge
        return { 0.0f, std::min((float)gridH - 1.0f, (gridH * 0.4f) * normY) };
    }
    // Right Screen Edge (Angle roughly between -45deg and 0deg)
    else if (angle > -0.25f * PI) {
        float normY = std::abs(angle) / (0.25f * PI); // 0.0 to 1.0 down right edge
        return { (float)(gridW - 1), std::min((float)gridH - 1.0f, (gridH * 0.4f) * normY) };
    }
    // Top Edge Arc (-135deg to -45deg)
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

static const int GRID_W = 90;
static const int GRID_H = 45;

struct RainDrop
{
    float x, y;
    float speed;
    float windFactor;
    int impactTimer;
};

struct ElectricParticle
{
    float x, y;
    float vx, vy;
    float alpha;
    char symbol;
};

struct LightningAnchor
{
    Vector2 start;
    Vector2 end;
};

static TitleTheme currentTitleTheme = THEME_FIRE;
static int titleMenuSelection = 0;
static std::vector<int> fireGrid;
static std::vector<RainDrop> rainDrops;
static std::vector<Snowflake> snowflakes;
static std::vector<int> snowAccumulation;
static std::vector<ElectricParticle> arcParticles;
static float electricCycleTimer = 0.0f;
static float currentWind = 0.35f; // Positive = wind blowing Right (\), Negative = Left (/), 0 = Straight down (|)
static std::vector<LightningAnchor> persistentAnchors;

void ResetTitleScreen()
{
    currentTitleTheme = (TitleTheme)GetRandomValue(0, THEME_COUNT - 1);
    electricCycleTimer = 0.0f;
    persistentAnchors.clear();
    std::fill(snowAccumulation.begin(), snowAccumulation.end(), 0);
}

void InitTitleScreen()
{
    titleMenuSelection = 0;
    fireGrid.assign(GRID_W * GRID_H, 0);
    snowAccumulation.assign(GRID_W * GRID_H, 0);

    rainDrops.resize(60);
    for (auto& drop : rainDrops)
    {
        drop.x = (float)GetRandomValue(0, GRID_W - 1);
        drop.y = (float)GetRandomValue(0, GRID_H - 1);
        drop.speed = (float)GetRandomValue(12, 22) / 10.0f; // Varied speed (1.2 to 2.2)
        drop.windFactor = (float)GetRandomValue(8, 13) / 10.0f; // Varied wind strength
        drop.impactTimer = 0;
    }

    const char flakeChars[] = "*+.o#";
    snowflakes.resize(50);
    for (auto& flake : snowflakes)
    {
        flake.x = (float)GetRandomValue(0, GRID_W - 1);
        flake.y = (float)GetRandomValue(0, GRID_H - 1);
        flake.speed = (float)GetRandomValue(2, 6) / 10.0f;
        flake.symbol = flakeChars[GetRandomValue(0, 4)];
        if (GetRandomValue(0, 1) == 0)
        {
            flake.color = SKYBLUE;
        }
        else
        {
            flake.color = RAYWHITE;
        }
    }

    const char sparkChars[] = "*+~:-";
    arcParticles.resize(35);
    for (auto& p : arcParticles)
    {
        p.x = (float)GetRandomValue(0, GRID_W - 1);
        p.y = (float)GetRandomValue(0, GRID_H - 1);
        p.vx = (float)GetRandomValue(-8, 8) / 10.0f;
        p.vy = (float)GetRandomValue(-8, 8) / 10.0f;
        p.alpha = (float)GetRandomValue(30, 100) / 100.0f;
        p.symbol = sparkChars[GetRandomValue(0, 4)];
    }

    ResetTitleScreen();
}

TitleAction UpdateAndDrawTitleScreen()
{
    float frameDt = GetFrameTime();
    TitleAction result = TITLE_ACTION_NONE;
    UpdateFire(fireGrid, GRID_W, GRID_H);

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
        if (titleMenuSelection == 0)
        {
            result = TITLE_ACTION_NEW_GAME;
        }
        else if (titleMenuSelection == 1)
        {
            result = TITLE_ACTION_OPTIONS;
        }
        else if (titleMenuSelection == 2)
        {
            result = TITLE_ACTION_QUIT;
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

        // --- Dynamic Wind Controls ---
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  currentWind -= 0.8f * frameDt;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) currentWind += 0.8f * frameDt;

        // Optional: Clamp wind strength so rain doesn't fly sideways completely
        currentWind = std::max(-1.5f, std::min(1.5f, currentWind));

        // --- Update Rain Drop Movement ---
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
            drop.x += currentWind * drop.windFactor; // Apply individual wind drift

            // Wrap around screen edges smoothly
            if (drop.x >= GRID_W) drop.x -= GRID_W;
            if (drop.x < 0)       drop.x += GRID_W;

            int currX = (int)drop.x;
            int currY = (int)drop.y;

            if (IsBannerRoofPixel(currX, currY)) {
                drop.y = (float)currY;
                drop.impactTimer = 4;
            }
            else if (drop.y >= GRID_H) {
                drop.y = 0;
                drop.x = (float)GetRandomValue(0, GRID_W - 1);
                drop.speed = (float)GetRandomValue(12, 22) / 10.0f; // Randomize speed on respawn
            }
        }

        // --- Render Rain Drops & Splashes ---
        for (const auto& drop : rainDrops) {
            int rx = (int)drop.x;
            int ry = (int)drop.y;

            if (drop.impactTimer > 0) {
                // Splash character on roof hit
                char splashStr[2] = { (GetRandomValue(0, 1) == 0) ? 'v' : '.', '\0' };
                DrawText(splashStr, (int)(rx * cellW), (int)(ry * cellH), (int)cellH, SKYBLUE);
            }
            else if (!IsBannerPixel(rx, ry)) {
                // Rain streak char matches wind direction (\, |, or /)
                char charToDraw = '|';
                if (currentWind > 0.3f) charToDraw = '\\';
                else if (currentWind < -0.3f) charToDraw = '/';

                char rainStr[2] = { charToDraw, '\0' };
                DrawText(rainStr, (int)(rx * cellW), (int)(ry * cellH), (int)cellH, DARKBLUE);
            }
        }

        // --- Render Banner for Rain Theme ---
        for (int y = 0; y < GRID_H; y++) {
            for (int x = 0; x < GRID_W; x++) {
                if (IsBannerPixel(x, y)) {
                    int bx = x - startX, by = y - startY;
                    char c = G_TITLE_BANNER[by][bx];
                    char str[2] = { c, '\0' };
                    DrawText(str, (int)(x * cellW), (int)(y * cellH), (int)cellH, BLUE);
                }
            }
        }
    } // Closes THEME_RAIN section
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
                if (i == 0)
                {
                    result = TITLE_ACTION_NEW_GAME;
                }
                else if (i == 1)
                {
                    result = TITLE_ACTION_OPTIONS;
                }
                else if (i == 2)
                {
                    result = TITLE_ACTION_QUIT;
                }
            }
        }

        Color col = (i == titleMenuSelection) ? YELLOW : WHITE;
        if (i == titleMenuSelection) DrawText(">", menuX - 20, optY, 24, YELLOW);
        DrawText(options[i], menuX, optY, 24, col);
    }

    EndDrawing();

    return result;
}
