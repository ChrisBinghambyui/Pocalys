#pragma once
#include <functional>
#include <vector>
#include "raylib.h"

// A walkable rectangle of tiles, covering [x, x + width) by [y, y + height).
struct NavRect
{
    int x;
    int y;
    int width;
    int height;
};

// A doorway from one rect into a neighbor. The midpoint is where paths cross between them.
struct NavPortal
{
    int toRect;
    float midX;
    float midY;
};

// A navmesh built from nothing but "is this tile walkable". It knows nothing about rooms, corridors,
// or the generator, so cave-style or hand-made maps work unchanged. Rebuild after the map changes.
//
// All positions in this class are tile-space CENTERS, so an enemy at Enemy.x/y is at (x + 0.5, y + 0.5).
class NavMesh
{
public:
    void Build(int width, int height, const std::function<bool(int, int)>& isWalkableTile);

    int GetRectCount() const;

    // Index of the rect containing the point, or -1 if the point is in a wall or off the map.
    int FindRectAt(float x, float y) const;

    // True if a body of this radius can walk the straight line without touching a wall.
    // Uses the same four-point wall check as player and enemy movement.
    bool HasClearLine(float x1, float y1, float x2, float y2, float bodyRadius) const;

    // Waypoints from start to goal (start itself is not included, the goal is the last point).
    // Empty if there is no route. rectExtraCost is an optional per-rect multiplier (0 = free, 1 = double cost)
    // so squads can steer members onto different routes later.
    std::vector<Vector2> FindPath(float startX, float startY, float goalX, float goalY, float bodyRadius, const std::vector<float>* rectExtraCost = nullptr) const;

    // Debug overlay: rect outlines and portal midpoints. Call inside BeginMode2D.
    void DrawDebug(int tileSize) const;

private:
    int _width = 0;
    int _height = 0;
    std::vector<char> _walkable;
    std::vector<int> _tileToRect;
    std::vector<NavRect> _rects;
    std::vector<std::vector<NavPortal>> _portals;

    bool IsTileWalkable(int tileX, int tileY) const;
    int GetRectAtTile(int tileX, int tileY) const;
    bool IsBodyClear(float centerX, float centerY, float radius) const;
    int FindNearestRect(float x, float y) const;
    void BuildRects();
    void BuildPortals();
    void AddSidePortals(int rectIndex, int outsideLine, float boundary, int start, int length, bool verticalSide);
};