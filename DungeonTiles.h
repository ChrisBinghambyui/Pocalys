#pragma once
#include <raylib.h>
#include "LightingShader.h"

// The 3D models used to draw floors and walls. Each model is auto-scaled so its longest side is one tile,
// centered on the tile, floor top at y = 0, wall bottom at y = 0.
struct TileKit
{
    Model floorModel = { 0 };
    Model wallModel = { 0 };
    Model cornerModel = { 0 };
    float floorScale = 1.0f;
    float wallScale = 1.0f;
    float cornerScale = 1.0f;
    bool floorReady = false;
    bool wallReady = false;
    bool cornerReady = false;
};

// Call once after InitLighting. Assigns the lighting shader to the kit materials.
// Returns true only if both models loaded. On false, callers should keep drawing cubes.
bool LoadTileKit(TileKit& kit, const LightingState& lighting);

void UnloadTileKit(TileKit& kit);

// tileX/tileY are map coordinates. Call inside BeginMode3D, after BeginShaderMode.
void DrawKitFloor(const TileKit& kit, LightingState& lighting, int tileX, int tileY);

// openDirX/openDirY is the cardinal direction toward the open neighbor this face looks at, e.g. (0, 1).
// Pass (0, 0) for a wall with no cardinal opening (corner-only exposure).
void DrawKitWall(const TileKit& kit, LightingState& lighting, int tileX, int tileY, int openDirX, int openDirY);

// openDirX/openDirY is the diagonal toward the open tile, e.g. (1, 1). Falls back to nothing if the corner model did not load.
void DrawKitCorner(const TileKit& kit, LightingState& lighting, int tileX, int tileY, int openDirX, int openDirY, float extraYaw = 0.0f, float pull = -1.0f);

extern float CORNER_YAW_OFFSET;
extern float CORNER_PULL;
extern float CONVEX_PULL;
extern float CONVEX_EXTRA_YAW;

// Call after the last kit model is drawn, before drawing cubes again.
extern float CORNER_YAW_OFFSET;
extern float CORNER_PULL;
extern float CONVEX_PULL;
extern float CONVEX_EXTRA_YAW;
void ResetKitLighting(LightingState& lighting);