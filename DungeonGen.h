#pragma once
#include <vector>
#include "DungeonData.h" // TileType, Room

// Builds a floor layout: varied room shapes (rectangles, circles and ovals, lopsided, cross, L, cavern),
// joined by winding corridors of mixed width, plus a few extra links so the map has loops.
//
// outTiles is sized width * height and indexed as outTiles[x * height + y], the same order as map[x][y].
// outRooms holds one bounding box per room. Every room's center tile and the 3x3 around it are always
// open floor, so stairs, patrol goals, and group spawn slots (all keyed off centerX/centerY) stay valid.
// Stairs and extraction tiles are NOT placed here, the caller does that.
void BuildDungeonLayout(int width, int height, int maxRooms, std::vector<TileType>& outTiles, std::vector<Room>& outRooms);