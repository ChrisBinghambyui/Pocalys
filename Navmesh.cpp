#include "NavMesh.h"
#include <cmath>
#include <queue>
#include <utility>
#include <functional>

static const float CLEAR_LINE_SAMPLE_STEP = 0.15f;

static float DistanceBetweenPoints(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

// ---------- Build ----------

void NavMesh::Build(int width, int height, const std::function<bool(int, int)>& isWalkableTile)
{
    _width = width;
    _height = height;
    _walkable.assign(width * height, 0);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (isWalkableTile(x, y))
            {
                _walkable[y * width + x] = 1;
            }
        }
    }

    BuildRects();
    BuildPortals();
}

int NavMesh::GetRectCount() const
{
    return (int)_rects.size();
}

bool NavMesh::IsTileWalkable(int tileX, int tileY) const
{
    if (tileX < 0 || tileX >= _width || tileY < 0 || tileY >= _height)
    {
        return false;
    }
    return _walkable[tileY * _width + tileX] != 0;
}

int NavMesh::GetRectAtTile(int tileX, int tileY) const
{
    if (tileX < 0 || tileX >= _width || tileY < 0 || tileY >= _height)
    {
        return -1;
    }
    return _tileToRect[tileY * _width + tileX];
}

// Greedy scan: from each unclaimed walkable tile, stretch right as far as it goes, then grow down
// while the whole row below is free. Every rect is convex by construction.
void NavMesh::BuildRects()
{
    _rects.clear();
    _tileToRect.assign(_width * _height, -1);

    for (int y = 0; y < _height; y++)
    {
        for (int x = 0; x < _width; x++)
        {
            if (!IsTileWalkable(x, y) || _tileToRect[y * _width + x] != -1)
            {
                continue;
            }

            int rectWidth = 1;
            while (x + rectWidth < _width && IsTileWalkable(x + rectWidth, y) && _tileToRect[y * _width + x + rectWidth] == -1)
            {
                rectWidth++;
            }

            int rectHeight = 1;
            bool canGrow = true;
            while (canGrow && y + rectHeight < _height)
            {
                for (int i = 0; i < rectWidth; i++)
                {
                    int checkX = x + i;
                    int checkY = y + rectHeight;
                    if (!IsTileWalkable(checkX, checkY) || _tileToRect[checkY * _width + checkX] != -1)
                    {
                        canGrow = false;
                        break;
                    }
                }
                if (canGrow)
                {
                    rectHeight++;
                }
            }

            NavRect rect = { x, y, rectWidth, rectHeight };
            int rectIndex = (int)_rects.size();
            _rects.push_back(rect);
            for (int ry = 0; ry < rectHeight; ry++)
            {
                for (int rx = 0; rx < rectWidth; rx++)
                {
                    _tileToRect[(y + ry) * _width + (x + rx)] = rectIndex;
                }
            }
        }
    }
}

// Walks the tiles just outside one side of a rect. Each run of tiles belonging to the same neighbor
// becomes one portal. A run is a single stretch because rects are rectangles.
void NavMesh::AddSidePortals(int rectIndex, int outsideLine, float boundary, int start, int length, bool verticalSide)
{
    int runRect = -1;
    int runStart = 0;

    for (int i = 0; i <= length; i++)
    {
        int cellRect = -1;
        if (i < length)
        {
            if (verticalSide)
            {
                cellRect = GetRectAtTile(outsideLine, start + i);
            }
            else
            {
                cellRect = GetRectAtTile(start + i, outsideLine);
            }
        }

        if (cellRect != runRect)
        {
            if (runRect >= 0)
            {
                float runMiddle = (float)start + (float)runStart + (float)(i - runStart) * 0.5f;
                NavPortal portal;
                portal.toRect = runRect;
                if (verticalSide)
                {
                    portal.midX = boundary;
                    portal.midY = runMiddle;
                }
                else
                {
                    portal.midX = runMiddle;
                    portal.midY = boundary;
                }
                _portals[rectIndex].push_back(portal);
            }
            runRect = cellRect;
            runStart = i;
        }
    }
}

void NavMesh::BuildPortals()
{
    _portals.assign(_rects.size(), std::vector<NavPortal>());

    for (size_t r = 0; r < _rects.size(); r++)
    {
        const NavRect& rect = _rects[r];
        int index = (int)r;
        AddSidePortals(index, rect.x + rect.width, (float)(rect.x + rect.width), rect.y, rect.height, true);
        AddSidePortals(index, rect.x - 1, (float)rect.x, rect.y, rect.height, true);
        AddSidePortals(index, rect.y + rect.height, (float)(rect.y + rect.height), rect.x, rect.width, false);
        AddSidePortals(index, rect.y - 1, (float)rect.y, rect.x, rect.width, false);
    }
}

// ---------- Queries ----------

int NavMesh::FindRectAt(float x, float y) const
{
    return GetRectAtTile((int)floorf(x), (int)floorf(y));
}

// Like FindRectAt, but a point inside a wall (noclip, a nudge off the edge) snaps to the closest rect.
int NavMesh::FindNearestRect(float x, float y) const
{
    int direct = FindRectAt(x, y);
    if (direct >= 0)
    {
        return direct;
    }

    int best = -1;
    float bestDistance = 1000000.0f;
    for (size_t i = 0; i < _rects.size(); i++)
    {
        const NavRect& rect = _rects[i];
        float clampedX = x;
        if (clampedX < (float)rect.x)
        {
            clampedX = (float)rect.x;
        }
        if (clampedX > (float)(rect.x + rect.width))
        {
            clampedX = (float)(rect.x + rect.width);
        }
        float clampedY = y;
        if (clampedY < (float)rect.y)
        {
            clampedY = (float)rect.y;
        }
        if (clampedY > (float)(rect.y + rect.height))
        {
            clampedY = (float)(rect.y + rect.height);
        }
        float distance = DistanceBetweenPoints(x, y, clampedX, clampedY);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = (int)i;
        }
    }
    return best;
}

bool NavMesh::IsBodyClear(float centerX, float centerY, float radius) const
{
    float checkPoints[4][2] = {
        { centerX - radius, centerY },
        { centerX + radius, centerY },
        { centerX, centerY - radius },
        { centerX, centerY + radius }
    };

    for (int i = 0; i < 4; i++)
    {
        if (!IsTileWalkable((int)floorf(checkPoints[i][0]), (int)floorf(checkPoints[i][1])))
        {
            return false;
        }
    }
    return true;
}

bool NavMesh::HasClearLine(float x1, float y1, float x2, float y2, float bodyRadius) const
{
    float length = DistanceBetweenPoints(x1, y1, x2, y2);
    int steps = (int)(length / CLEAR_LINE_SAMPLE_STEP) + 1;

    for (int i = 0; i <= steps; i++)
    {
        float t = (float)i / (float)steps;
        float sampleX = x1 + (x2 - x1) * t;
        float sampleY = y1 + (y2 - y1) * t;
        if (!IsBodyClear(sampleX, sampleY, bodyRadius))
        {
            return false;
        }
    }
    return true;
}

// A* across rects. A rect's position for cost purposes is the portal it was entered through, so a
// long corridor costs its real length instead of center-to-center guesses. The raw portal midpoints
// are then pulled taut: from each point, jump to the furthest waypoint a body can walk to directly.
std::vector<Vector2> NavMesh::FindPath(float startX, float startY, float goalX, float goalY, float bodyRadius, const std::vector<float>* rectExtraCost) const
{
    std::vector<Vector2> result;
    if (_rects.empty())
    {
        return result;
    }

    int startRect = FindNearestRect(startX, startY);
    int goalRect = FindNearestRect(goalX, goalY);
    if (startRect < 0 || goalRect < 0)
    {
        return result;
    }

    Vector2 goal = { goalX, goalY };
    if (startRect == goalRect)
    {
        result.push_back(goal);
        return result;
    }

    int rectCount = (int)_rects.size();
    std::vector<float> bestCost(rectCount, 1000000.0f);
    std::vector<int> cameFrom(rectCount, -1);
    std::vector<Vector2> entryPoint(rectCount);
    std::vector<char> closed(rectCount, 0);

    typedef std::pair<float, int> OpenNode;
    std::priority_queue<OpenNode, std::vector<OpenNode>, std::greater<OpenNode>> open;

    entryPoint[startRect] = { startX, startY };
    bestCost[startRect] = 0.0f;
    open.push(OpenNode(DistanceBetweenPoints(startX, startY, goalX, goalY), startRect));

    bool found = false;
    while (!open.empty())
    {
        OpenNode top = open.top();
        open.pop();
        int current = top.second;
        if (closed[current] != 0)
        {
            continue;
        }
        closed[current] = 1;
        if (current == goalRect)
        {
            found = true;
            break;
        }

        for (size_t p = 0; p < _portals[current].size(); p++)
        {
            const NavPortal& portal = _portals[current][p];
            if (closed[portal.toRect] != 0)
            {
                continue;
            }

            float stepCost = DistanceBetweenPoints(entryPoint[current].x, entryPoint[current].y, portal.midX, portal.midY);
            if (rectExtraCost != nullptr && portal.toRect < (int)rectExtraCost->size())
            {
                stepCost = stepCost * (1.0f + (*rectExtraCost)[portal.toRect]);
            }

            float newCost = bestCost[current] + stepCost;
            if (newCost < bestCost[portal.toRect])
            {
                bestCost[portal.toRect] = newCost;
                cameFrom[portal.toRect] = current;
                entryPoint[portal.toRect] = { portal.midX, portal.midY };
                float estimate = newCost + DistanceBetweenPoints(portal.midX, portal.midY, goalX, goalY);
                open.push(OpenNode(estimate, portal.toRect));
            }
        }
    }

    if (!found)
    {
        return result;
    }

    // Walk back from the goal rect collecting entry points, then flip into start-to-goal order
    std::vector<Vector2> waypoints;
    int cursor = goalRect;
    while (cursor != startRect)
    {
        waypoints.push_back(entryPoint[cursor]);
        cursor = cameFrom[cursor];
    }
    std::vector<Vector2> ordered;
    for (int i = (int)waypoints.size() - 1; i >= 0; i--)
    {
        ordered.push_back(waypoints[i]);
    }
    ordered.push_back(goal);

    // Pull taut
    float currentX = startX;
    float currentY = startY;
    int index = 0;
    while (index < (int)ordered.size())
    {
        int furthest = index;
        for (int j = (int)ordered.size() - 1; j > index; j--)
        {
            if (HasClearLine(currentX, currentY, ordered[j].x, ordered[j].y, bodyRadius))
            {
                furthest = j;
                break;
            }
        }
        result.push_back(ordered[furthest]);
        currentX = ordered[furthest].x;
        currentY = ordered[furthest].y;
        index = furthest + 1;
    }

    return result;
}

void NavMesh::DrawDebug(int tileSize) const
{
    for (size_t i = 0; i < _rects.size(); i++)
    {
        const NavRect& rect = _rects[i];
        DrawRectangleLines(rect.x * tileSize, rect.y * tileSize, rect.width * tileSize, rect.height * tileSize, Fade(SKYBLUE, 0.6f));
        for (size_t p = 0; p < _portals[i].size(); p++)
        {
            DrawCircle((int)(_portals[i][p].midX * tileSize), (int)(_portals[i][p].midY * tileSize), 3.0f, ORANGE);
        }
    }
}