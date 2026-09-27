#include "TargetingUI.h"
#include <raylib.h>
#include <cmath>

static int SignOf(int value)
{
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

static std::vector<TilePos> GetLineTiles(int casterX, int casterY, int hoverX, int hoverY, int range)
{
    std::vector<TilePos> tiles;
    int dx = SignOf(hoverX - casterX);
    int dy = SignOf(hoverY - casterY);
    if (dx == 0 && dy == 0)
    {
        return tiles;
    }

    int x = casterX;
    int y = casterY;
    for (int step = 0; step < range; step++)
    {
        x += dx;
        y += dy;
        tiles.push_back({ x, y });
    }
    return tiles;
}

static std::vector<TilePos> GetConeTiles(int casterX, int casterY, int hoverX, int hoverY, int range)
{
    std::vector<TilePos> tiles;
    float dirX = (float)(hoverX - casterX);
    float dirY = (float)(hoverY - casterY);
    float dirLength = std::sqrt(dirX * dirX + dirY * dirY);
    if (dirLength < 0.001f)
    {
        return tiles;
    }
    dirX /= dirLength;
    dirY /= dirLength;

    for (int checkX = casterX - range; checkX <= casterX + range; checkX++)
    {
        for (int checkY = casterY - range; checkY <= casterY + range; checkY++)
        {
            int offsetX = checkX - casterX;
            int offsetY = checkY - casterY;
            if (offsetX == 0 && offsetY == 0)
            {
                continue;
            }
            float dist = std::sqrt((float)(offsetX * offsetX + offsetY * offsetY));
            if (dist > (float)range)
            {
                continue;
            }
            float dot = (offsetX / dist) * dirX + (offsetY / dist) * dirY;
            if (dot >= 0.6f) // roughly a 105 degree cone
            {
                tiles.push_back({ checkX, checkY });
            }
        }
    }
    return tiles;
}

static std::vector<TilePos> GetAreaTiles(int hoverX, int hoverY, int radius)
{
    std::vector<TilePos> tiles;
    for (int checkX = hoverX - radius; checkX <= hoverX + radius; checkX++)
    {
        for (int checkY = hoverY - radius; checkY <= hoverY + radius; checkY++)
        {
            int offsetX = checkX - hoverX;
            int offsetY = checkY - hoverY;
            if (offsetX * offsetX + offsetY * offsetY <= radius * radius)
            {
                tiles.push_back({ checkX, checkY });
            }
        }
    }
    return tiles;
}

std::vector<TilePos> GetTargetedTiles(int casterX, int casterY, int hoverX, int hoverY, const AbilityDef& ability)
{
    if (ability.shape == ABILITY_SHAPE_SELF)
    {
        return { { casterX, casterY } };
    }
    if (ability.shape == ABILITY_SHAPE_ADJACENT)
    {
        int dx = SignOf(hoverX - casterX);
        int dy = SignOf(hoverY - casterY);
        if (dx == 0 && dy == 0)
        {
            return {};
        }
        return { { casterX + dx, casterY + dy } };
    }
    if (ability.shape == ABILITY_SHAPE_LINE)
    {
        return GetLineTiles(casterX, casterY, hoverX, hoverY, ability.range);
    }
    if (ability.shape == ABILITY_SHAPE_CONE)
    {
        return GetConeTiles(casterX, casterY, hoverX, hoverY, ability.range);
    }
    if (ability.shape == ABILITY_SHAPE_RANGED_SINGLE)
    {
        std::vector<TilePos> line = GetLineTiles(casterX, casterY, hoverX, hoverY, ability.range);
        if (line.empty())
        {
            return line;
        }
        return { line.back() };
    }
    if (ability.shape == ABILITY_SHAPE_AREA)
    {
        return GetAreaTiles(hoverX, hoverY, 2); // Placeholder radius, no areaRadius field yet
    }
    return {};
}

void DrawTargetingOverlay(int casterX, int casterY, int hoverX, int hoverY, const AbilityDef& ability, int tileSize)
{
    std::vector<TilePos> tiles = GetTargetedTiles(casterX, casterY, hoverX, hoverY, ability);
    for (size_t i = 0; i < tiles.size(); i++)
    {
        Rectangle tileRect = { (float)(tiles[i].x * tileSize), (float)(tiles[i].y * tileSize), (float)tileSize, (float)tileSize };
        DrawRectangleRec(tileRect, Fade(RED, 0.35f));
        DrawRectangleLinesEx(tileRect, 1.0f, Fade(RED, 0.8f));
    }
}