#include "DungeonGen.h"
#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

enum RoomShape
{
    SHAPE_RECT,
    SHAPE_ELLIPSE,  // Circles when the box is square
    SHAPE_LOPSIDED, // Two overlapping rectangles, stepped
    SHAPE_CROSS,
    SHAPE_L,
    SHAPE_CAVERN    // Ellipse with a wobbling, ragged edge
};

static const int ROOM_MIN_WIDTH = 8;
static const int ROOM_MAX_WIDTH = 16;
static const int ROOM_MIN_HEIGHT = 7;
static const int ROOM_MAX_HEIGHT = 13;
static const int ROOM_GAP = 3;              // Empty tiles kept between room bounding boxes
static const int PLACEMENT_ATTEMPTS = 300;
static const int EXTRA_LINK_CHANCE = 35;    // Percent chance a room gets a second corridor, which makes loops
static const int CORRIDOR_RUN_MIN = 3;
static const int CORRIDOR_RUN_MAX = 9;
static const int CORRIDOR_WOBBLE_CHANCE = 25; // Percent chance a run is a sideways detour instead of progress

// ---------- Tile helpers ----------

// Never touches the outermost ring of the map, so the border always stays wall.
static void CarveTile(std::vector<TileType>& tiles, int width, int height, int x, int y)
{
    if (x < 1 || x >= width - 1 || y < 1 || y >= height - 1)
    {
        return;
    }
    tiles[x * height + y] = TILE_FLOOR;
}

static void CarveRect(std::vector<TileType>& tiles, int width, int height, int rectX, int rectY, int rectW, int rectH)
{
    for (int x = rectX; x < rectX + rectW; x++)
    {
        for (int y = rectY; y < rectY + rectH; y++)
        {
            CarveTile(tiles, width, height, x, y);
        }
    }
}

// Square brush centered on (centerX, centerY). Width 1 is a single tile, 2 and 3 give wide hallways.
static void CarveBrush(std::vector<TileType>& tiles, int width, int height, int centerX, int centerY, int brushWidth)
{
    int start = -(brushWidth / 2);
    for (int dx = 0; dx < brushWidth; dx++)
    {
        for (int dy = 0; dy < brushWidth; dy++)
        {
            CarveTile(tiles, width, height, centerX + start + dx, centerY + start + dy);
        }
    }
}

// ---------- Room shapes ----------

static void CarveEllipse(std::vector<TileType>& tiles, int width, int height, const Room& room)
{
    float halfW = (float)room.width / 2.0f;
    float halfH = (float)room.height / 2.0f;
    float centerX = (float)room.x + halfW;
    float centerY = (float)room.y + halfH;

    for (int x = room.x; x < room.x + room.width; x++)
    {
        for (int y = room.y; y < room.y + room.height; y++)
        {
            float nx = ((float)x + 0.5f - centerX) / halfW;
            float ny = ((float)y + 0.5f - centerY) / halfH;
            if (nx * nx + ny * ny <= 1.0f)
            {
                CarveTile(tiles, width, height, x, y);
            }
        }
    }
}

static void CarveCavern(std::vector<TileType>& tiles, int width, int height, const Room& room)
{
    float halfW = (float)room.width / 2.0f;
    float halfH = (float)room.height / 2.0f;
    float centerX = (float)room.x + halfW;
    float centerY = (float)room.y + halfH;

    float phaseA = (float)GetRandomValue(0, 628) / 100.0f;
    float phaseB = (float)GetRandomValue(0, 628) / 100.0f;
    float freqA = (float)GetRandomValue(2, 4);
    float freqB = (float)GetRandomValue(5, 7);

    for (int x = room.x; x < room.x + room.width; x++)
    {
        for (int y = room.y; y < room.y + room.height; y++)
        {
            float nx = ((float)x + 0.5f - centerX) / halfW;
            float ny = ((float)y + 0.5f - centerY) / halfH;
            float angle = atan2f(ny, nx);
            float edge = 0.85f + 0.09f * sinf(freqA * angle + phaseA) + 0.06f * sinf(freqB * angle + phaseB);
            float dist = sqrtf(nx * nx + ny * ny);

            if (dist <= edge)
            {
                CarveTile(tiles, width, height, x, y);
            }
            else if (dist <= edge + 0.08f && GetRandomValue(0, 2) == 0)
            {
                CarveTile(tiles, width, height, x, y); // Ragged fringe
            }
        }
    }
}

// Two overlapping rectangles pushed to opposite corners of the box. Both are over half the box in each
// direction, so they always overlap and the box center is always inside at least one of them.
static void CarveLopsided(std::vector<TileType>& tiles, int width, int height, const Room& room)
{
    int aw = room.width * GetRandomValue(58, 75) / 100;
    int ah = room.height * GetRandomValue(58, 75) / 100;
    int bw = room.width * GetRandomValue(58, 75) / 100;
    int bh = room.height * GetRandomValue(58, 75) / 100;

    int ax = room.x;
    int bx = room.x + room.width - bw;
    if (GetRandomValue(0, 1) == 0)
    {
        ax = room.x + room.width - aw;
        bx = room.x;
    }
    int ay = room.y;
    int by = room.y + room.height - bh;

    CarveRect(tiles, width, height, ax, ay, aw, ah);
    CarveRect(tiles, width, height, bx, by, bw, bh);
}

static void CarveCross(std::vector<TileType>& tiles, int width, int height, const Room& room)
{
    int armW = std::max(3, room.width / 3);
    int armH = std::max(3, room.height / 3);
    CarveRect(tiles, width, height, room.x, room.y + (room.height - armH) / 2, room.width, armH);
    CarveRect(tiles, width, height, room.x + (room.width - armW) / 2, room.y, armW, room.height);
}

// Two arms that meet at the box center, each running out to a box edge. Random corner each time.
static void CarveLShape(std::vector<TileType>& tiles, int width, int height, const Room& room)
{
    int thickness = GetRandomValue(3, 5);
    int half = thickness / 2;
    int centerX = room.x + room.width / 2;
    int centerY = room.y + room.height / 2;
    int cornerX = centerX - half;
    int cornerY = centerY - half;

    bool armLeft = (GetRandomValue(0, 1) == 0);
    bool armUp = (GetRandomValue(0, 1) == 0);

    int horizontalX = cornerX;
    int horizontalW = room.x + room.width - cornerX;
    if (armLeft)
    {
        horizontalX = room.x;
        horizontalW = cornerX + thickness - room.x;
    }

    int verticalY = cornerY;
    int verticalH = room.y + room.height - cornerY;
    if (armUp)
    {
        verticalY = room.y;
        verticalH = cornerY + thickness - room.y;
    }

    CarveRect(tiles, width, height, horizontalX, cornerY, horizontalW, thickness);
    CarveRect(tiles, width, height, cornerX, verticalY, thickness, verticalH);
}

static RoomShape RollShape()
{
    int roll = GetRandomValue(1, 100);
    if (roll <= 25)
    {
        return SHAPE_RECT;
    }
    if (roll <= 45)
    {
        return SHAPE_ELLIPSE;
    }
    if (roll <= 65)
    {
        return SHAPE_LOPSIDED;
    }
    if (roll <= 75)
    {
        return SHAPE_CROSS;
    }
    if (roll <= 85)
    {
        return SHAPE_L;
    }
    return SHAPE_CAVERN;
}

static void CarveRoomShape(std::vector<TileType>& tiles, int width, int height, const Room& room, RoomShape shape)
{
    if (shape == SHAPE_RECT)
    {
        CarveRect(tiles, width, height, room.x, room.y, room.width, room.height);
    }
    else if (shape == SHAPE_ELLIPSE)
    {
        CarveEllipse(tiles, width, height, room);
    }
    else if (shape == SHAPE_LOPSIDED)
    {
        CarveLopsided(tiles, width, height, room);
    }
    else if (shape == SHAPE_CROSS)
    {
        CarveCross(tiles, width, height, room);
    }
    else if (shape == SHAPE_L)
    {
        CarveLShape(tiles, width, height, room);
    }
    else
    {
        CarveCavern(tiles, width, height, room);
    }

    // Stairs, patrol goals and the 3x3 group spawn slots all hang off the box center, so it is always open
    CarveRect(tiles, width, height, room.centerX() - 1, room.centerY() - 1, 3, 3);
}

// ---------- Corridors ----------

static int Sign(int value)
{
    if (value > 0)
    {
        return 1;
    }
    if (value < 0)
    {
        return -1;
    }
    return 0;
}

static int ClampInt(int value, int low, int high)
{
    if (value < low)
    {
        return low;
    }
    if (value > high)
    {
        return high;
    }
    return value;
}

// Walks from the start to the end in straight runs of a few tiles. Each run either makes progress along one
// axis or is a short sideways detour, so the path snakes instead of forming a clean L. A step budget stops
// runaway wandering, and a plain L-shaped finish guarantees the two ends always join.
static void CarveWindingCorridor(std::vector<TileType>& tiles, int width, int height, int startX, int startY, int endX, int endY, int brushWidth)
{
    int x = startX;
    int y = startY;
    int stepBudget = (std::abs(endX - startX) + std::abs(endY - startY)) * 4 + 50;
    CarveBrush(tiles, width, height, x, y, brushWidth);

    while ((x != endX || y != endY) && stepBudget > 0)
    {
        int remainX = endX - x;
        int remainY = endY - y;
        int totalRemain = std::abs(remainX) + std::abs(remainY);

        bool moveOnX = false;
        if (remainY == 0)
        {
            moveOnX = true;
        }
        else if (remainX != 0)
        {
            if (GetRandomValue(1, totalRemain) <= std::abs(remainX))
            {
                moveOnX = true;
            }
        }

        int dirX = 0;
        int dirY = 0;
        int runLength = GetRandomValue(CORRIDOR_RUN_MIN, CORRIDOR_RUN_MAX);
        bool wobble = (GetRandomValue(1, 100) <= CORRIDOR_WOBBLE_CHANCE);

        if (wobble)
        {
            int side = 1;
            if (GetRandomValue(0, 1) == 0)
            {
                side = -1;
            }
            if (moveOnX)
            {
                dirY = side;
            }
            else
            {
                dirX = side;
            }
            runLength = GetRandomValue(2, 4);
        }
        else
        {
            if (moveOnX)
            {
                dirX = Sign(remainX);
                runLength = std::min(runLength, std::abs(remainX));
            }
            else
            {
                dirY = Sign(remainY);
                runLength = std::min(runLength, std::abs(remainY));
            }
        }

        for (int step = 0; step < runLength; step++)
        {
            x = ClampInt(x + dirX, 1, width - 2);
            y = ClampInt(y + dirY, 1, height - 2);
            CarveBrush(tiles, width, height, x, y, brushWidth);
            stepBudget--;
        }
    }

    while (x != endX)
    {
        x += Sign(endX - x);
        CarveBrush(tiles, width, height, x, y, brushWidth);
    }
    while (y != endY)
    {
        y += Sign(endY - y);
        CarveBrush(tiles, width, height, x, y, brushWidth);
    }
}

static int RollCorridorWidth()
{
    int roll = GetRandomValue(1, 100);
    if (roll <= 40)
    {
        return 1;
    }
    if (roll <= 80)
    {
        return 2;
    }
    return 3;
}

static int SquaredCenterDistance(const Room& a, const Room& b)
{
    int dx = a.centerX() - b.centerX();
    int dy = a.centerY() - b.centerY();
    return dx * dx + dy * dy;
}

static void ConnectRooms(std::vector<TileType>& tiles, int width, int height, const Room& a, const Room& b)
{
    CarveWindingCorridor(tiles, width, height, a.centerX(), a.centerY(), b.centerX(), b.centerY(), RollCorridorWidth());
}

// ---------- Entry point ----------

void BuildDungeonLayout(int width, int height, int maxRooms, std::vector<TileType>& outTiles, std::vector<Room>& outRooms)
{
    outTiles.assign(width * height, TILE_WALL);
    outRooms.clear();

    int attempts = 0;
    while ((int)outRooms.size() < maxRooms && attempts < PLACEMENT_ATTEMPTS)
    {
        attempts++;

        RoomShape shape = RollShape();
        int roomW = GetRandomValue(ROOM_MIN_WIDTH, ROOM_MAX_WIDTH);
        int roomH = GetRandomValue(ROOM_MIN_HEIGHT, ROOM_MAX_HEIGHT);

        if (shape == SHAPE_ELLIPSE && GetRandomValue(0, 1) == 0)
        {
            roomW = GetRandomValue(9, ROOM_MAX_HEIGHT); // A true circle
            roomH = roomW;
        }
        if (shape == SHAPE_CROSS || shape == SHAPE_L)
        {
            roomW = std::max(roomW, 10);
            roomH = std::max(roomH, 10);
        }

        int roomX = GetRandomValue(2, width - roomW - 3);
        int roomY = GetRandomValue(2, height - roomH - 3);
        Room candidate = { roomX, roomY, roomW, roomH };

        Room padded = { roomX - ROOM_GAP, roomY - ROOM_GAP, roomW + ROOM_GAP * 2, roomH + ROOM_GAP * 2 };
        bool blocked = false;
        for (size_t i = 0; i < outRooms.size(); i++)
        {
            if (padded.intersects(outRooms[i]))
            {
                blocked = true;
                break;
            }
        }
        if (blocked)
        {
            continue;
        }

        CarveRoomShape(outTiles, width, height, candidate, shape);
        outRooms.push_back(candidate);
    }

    int roomCount = (int)outRooms.size();
    if (roomCount < 2)
    {
        return;
    }

    std::vector<std::vector<bool>> linked(roomCount, std::vector<bool>(roomCount, false));

    // Every room joins the nearest room placed before it, which keeps the whole map connected
    for (int i = 1; i < roomCount; i++)
    {
        int nearest = 0;
        int nearestDistance = SquaredCenterDistance(outRooms[i], outRooms[0]);
        for (int j = 1; j < i; j++)
        {
            int distance = SquaredCenterDistance(outRooms[i], outRooms[j]);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearest = j;
            }
        }
        ConnectRooms(outTiles, width, height, outRooms[i], outRooms[nearest]);
        linked[i][nearest] = true;
        linked[nearest][i] = true;
    }

    // Some rooms get a second link to their nearest unlinked neighbor, so there are loops and flanking routes
    for (int i = 0; i < roomCount; i++)
    {
        if (GetRandomValue(1, 100) > EXTRA_LINK_CHANCE)
        {
            continue;
        }

        int nearest = -1;
        int nearestDistance = 0;
        for (int j = 0; j < roomCount; j++)
        {
            if (j == i || linked[i][j])
            {
                continue;
            }
            int distance = SquaredCenterDistance(outRooms[i], outRooms[j]);
            if (nearest < 0 || distance < nearestDistance)
            {
                nearestDistance = distance;
                nearest = j;
            }
        }
        if (nearest < 0)
        {
            continue;
        }
        ConnectRooms(outTiles, width, height, outRooms[i], outRooms[nearest]);
        linked[i][nearest] = true;
        linked[nearest][i] = true;
    }
}