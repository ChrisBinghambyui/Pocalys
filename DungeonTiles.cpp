#include "DungeonTiles.h"
#include <raymath.h>
#include <rlgl.h>
#include <cmath>

// Change these to match the filenames in your assets folder.
static const char* KIT_FLOOR_PATH = "assets/models/dungeon/template-floor.glb";
static const char* KIT_WALL_PATH = "assets/models/dungeon/template-wall.glb";
static const char* KIT_CORNER_PATH = "assets/models/dungeon/template-corner.glb";

// Tuning for the wall panel. Walls are drawn once per open side, pushed toward that side.
// If panels look sunk into the wall tile, raise the offset. If they face the wrong way, change the yaw offset by 90 or 180.
static const float WALL_FACE_OFFSET = 0.0f; // Tile units, toward the open neighbor
static const float WALL_YAW_OFFSET = 0.0f;  // Degrees, added to the computed facing
float CORNER_YAW_OFFSET = 135.0f; // Degrees. If the corner piece points the wrong way, try 0, 180, or 270.
float CORNER_PULL = 0.6f;
float CONVEX_PULL = -0.2f;
float CONVEX_EXTRA_YAW = 180.0f;

// Loads one model, scales its longest horizontal side to one tile, and centers it on the origin.
// topAtOrigin true puts the top of the model at y = 0 (floors), false puts the bottom there (walls).
static bool LoadFittedModel(Model& model, const char* path, bool topAtOrigin, float& outScale, const LightingState& lighting)
{
    if (!FileExists(path))
    {
        return false;
    }

    model = LoadModel(path);
    if (model.meshCount <= 0)
    {
        return false;
    }

    BoundingBox box = GetModelBoundingBox(model);
    float sizeX = box.max.x - box.min.x;
    float sizeZ = box.max.z - box.min.z;
    float longest = sizeX;
    if (sizeZ > longest)
    {
        longest = sizeZ;
    }
    if (longest < 0.0001f)
    {
        UnloadModel(model);
        model = Model();
        return false;
    }

    outScale = 1.0f / longest;

    float centerX = (box.min.x + box.max.x) * 0.5f;
    float centerZ = (box.min.z + box.max.z) * 0.5f;
    float yShift = -box.min.y;
    if (topAtOrigin)
    {
        yShift = -box.max.y;
    }
    // Applied by DrawModelEx before its own scale and rotation, so these are in unscaled model units
    model.transform = MatrixTranslate(-centerX, yShift, -centerZ);

    if (lighting.ready)
    {
        for (int m = 0; m < model.materialCount; m++)
        {
            model.materials[m].shader = lighting.shader;
        }
    }
    return true;
}

bool LoadTileKit(TileKit& kit, const LightingState& lighting)
{
    kit.floorReady = LoadFittedModel(kit.floorModel, KIT_FLOOR_PATH, true, kit.floorScale, lighting);
    kit.wallReady = LoadFittedModel(kit.wallModel, KIT_WALL_PATH, false, kit.wallScale, lighting);
    kit.cornerReady = LoadFittedModel(kit.cornerModel, KIT_CORNER_PATH, false, kit.cornerScale, lighting);
    /*kit.cornerScale = kit.wallScale;*/
    return kit.floorReady && kit.wallReady;
}

void UnloadTileKit(TileKit& kit)
{
    if (kit.cornerReady)
    {
        UnloadModel(kit.cornerModel);
        kit.cornerReady = false;
    }
    if (kit.floorReady)
    {
        UnloadModel(kit.floorModel);
        kit.floorReady = false;
    }
    if (kit.wallReady)
    {
        UnloadModel(kit.wallModel);
        kit.wallReady = false;
    }
}

// The lighting shader works out world position from lightModelMatrix, so it has to match each draw.
static void DrawKitModel(LightingState& lighting, const Model& model, float scale, Vector3 position, float yawDegrees)
{
    Matrix scaleMatrix = MatrixScale(scale, scale, scale);
    Matrix rotateMatrix = MatrixRotateY(yawDegrees * DEG2RAD);
    Matrix moveMatrix = MatrixTranslate(position.x, position.y, position.z);
    Matrix worldMatrix = MatrixMultiply(MatrixMultiply(scaleMatrix, rotateMatrix), moveMatrix);
    worldMatrix = MatrixMultiply(model.transform, worldMatrix);
    SetLightingModelMatrix(lighting, worldMatrix);

    Vector3 yawAxis = { 0.0f, 1.0f, 0.0f };
    Vector3 scaleVector = { scale, scale, scale };
    DrawModelEx(model, position, yawAxis, yawDegrees, scaleVector, WHITE);
}

void DrawKitFloor(const TileKit& kit, LightingState& lighting, int tileX, int tileY)
{
    Vector3 position = { (float)tileX + 0.5f, 0.0f, (float)tileY + 0.5f };
    DrawKitModel(lighting, kit.floorModel, kit.floorScale, position, 0.0f);
}

void DrawKitWall(const TileKit& kit, LightingState& lighting, int tileX, int tileY, int openDirX, int openDirY)
{
    float yaw = WALL_YAW_OFFSET;
    float pushX = 0.0f;
    float pushZ = 0.0f;
    if (openDirX != 0 || openDirY != 0)
    {
        yaw += atan2f((float)openDirX, (float)openDirY) * RAD2DEG; // (0, 1) faces +Z, (1, 0) faces +X
        pushX = (float)openDirX * WALL_FACE_OFFSET;
        pushZ = (float)openDirY * WALL_FACE_OFFSET;
    }

    Vector3 position = { (float)tileX + 0.5f + pushX, 0.0f, (float)tileY + 0.5f + pushZ };
    DrawKitModel(lighting, kit.wallModel, kit.wallScale, position, yaw);
}

void DrawKitCorner(const TileKit& kit, LightingState& lighting, int tileX, int tileY, int openDirX, int openDirY, float extraYaw, float pull)
{
    if (!kit.cornerReady)
    {
        return;
    }

    float yaw = CORNER_YAW_OFFSET + extraYaw + atan2f((float)openDirX, (float)openDirY) * RAD2DEG;
    float usedPull = CORNER_PULL;
    if (pull > -0.5f)
    {
        usedPull = pull;
    }
    Vector3 position = { (float)tileX + 0.5f + (float)openDirX * usedPull, 0.0f, (float)tileY + 0.5f + (float)openDirY * usedPull };
    DrawKitModel(lighting, kit.cornerModel, kit.cornerScale, position, yaw);
}

void ResetKitLighting(LightingState& lighting)
{
    rlDrawRenderBatchActive();
    SetLightingModelMatrix(lighting, MatrixIdentity());
}