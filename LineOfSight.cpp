#include "LineOfSight.h"
#include <cstdlib>

bool HasLineOfSight(int x0, int y0, int x1, int y1, const TileOpaqueFn& isOpaque)
{
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int currX = x0;
    int currY = y0;

    while (currX != x1 || currY != y1)
    {
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            currX += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            currY += sy;
        }

        if (currX == x1 && currY == y1)
        {
            break; // Reached the target tile. Its own opacity never blocks sight of itself.
        }
        if (isOpaque(currX, currY))
        {
            return false;
        }
    }

    return true;
}
