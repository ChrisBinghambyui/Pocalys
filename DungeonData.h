#pragma once // This tells the compiler to only include this file once per build

#include <vector>
#include "Enemy.h"

// 1. Define TileType FIRST
enum TileType {
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIR_UP,
    TILE_STAIR_DOWN
};

const int mapWidth = 100;
const int mapHeight = 100;
const int maxRooms = 30; // More rooms for the bigger map!

// 2. NOW we can use TileType inside our struct
struct LevelState {
    TileType savedMap[mapWidth][mapHeight];
    bool savedExplored[mapWidth][mapHeight];
    std::vector<Enemy> savedEnemies; // Corpses and survivors both persist here
};

// A handy struct to keep track of our rooms and find their centers
struct Room {
    int x, y, width, height;

    int centerX() const { return x + width / 2; }
    int centerY() const { return y + height / 2; }

    // Check if this room's boundaries overlap with another's
    bool intersects(const Room& other) const {
        return (x <= other.x + other.width && x + width >= other.x &&
            y <= other.y + other.height && y + height >= other.y);
    }
};