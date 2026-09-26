#pragma once
#include <functional>

// True if (x,y) blocks sight. Walls should return true, open floor false.
using TileOpaqueFn = std::function<bool(int, int)>;

// Bresenham walk from (x0,y0) to (x1,y1). Blocked if any tile strictly between the two endpoints
// is opaque. The endpoints themselves never block: you can always see a wall's own face, and the
// looker's own tile never blocks its own sight.
bool HasLineOfSight(int x0, int y0, int x1, int y1, const TileOpaqueFn& isOpaque);
