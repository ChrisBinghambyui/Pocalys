#include "TargetingUI.h"
#include <raylib.h>
#include <cmath>

static const float REACH_PADDING = 0.2f;          // Center-to-center slack so a neighbor counts as inside range 1
static const float ADJACENT_HALF_ANGLE = 30.0f;   // Narrow, focused strike
static const float CONE_HALF_ANGLE = 60.0f;       // Wide sweep
static const float LINE_WIDTH = 0.8f;             // Thrusts and piercing lines, tiles
static const float RANGED_LANE_WIDTH = 0.7f;      // Hit lane of a shot, tiles. Drawn thinner than it hits.
static const float SELF_RADIUS = 0.6f;
static const float AREA_PLACEHOLDER_RADIUS = 2.0f; // No areaRadius field on AbilityDef yet

AimGeometry BuildAimGeometry(float casterX, float casterY, float aimX, float aimY, const AbilityDef& ability)
{
    AimGeometry geometry;
    geometry.shape = ability.shape;
    geometry.originX = casterX;
    geometry.originY = casterY;
    geometry.dirX = 1.0f;
    geometry.dirY = 0.0f;
    geometry.reach = 0.0f;
    geometry.width = 0.0f;
    geometry.halfAngleDeg = 0.0f;
    geometry.centerX = casterX;
    geometry.centerY = casterY;
    geometry.radius = 0.0f;

    float toAimX = aimX - casterX;
    float toAimY = aimY - casterY;
    float toAimLength = std::sqrt(toAimX * toAimX + toAimY * toAimY);
    if (toAimLength > 0.001f)
    {
        geometry.dirX = toAimX / toAimLength;
        geometry.dirY = toAimY / toAimLength;
    }

    if (ability.shape == ABILITY_SHAPE_SELF)
    {
        geometry.radius = SELF_RADIUS;
    }
    else if (ability.shape == ABILITY_SHAPE_ADJACENT)
    {
        geometry.reach = (float)ability.range + REACH_PADDING;
        geometry.halfAngleDeg = ADJACENT_HALF_ANGLE;
    }
    else if (ability.shape == ABILITY_SHAPE_CONE)
    {
        geometry.reach = (float)ability.range + REACH_PADDING;
        geometry.halfAngleDeg = CONE_HALF_ANGLE;
    }
    else if (ability.shape == ABILITY_SHAPE_LINE)
    {
        geometry.reach = (float)ability.range + REACH_PADDING;
        geometry.width = LINE_WIDTH;
    }
    else if (ability.shape == ABILITY_SHAPE_RANGED_SINGLE)
    {
        geometry.reach = (float)ability.range;
        geometry.width = RANGED_LANE_WIDTH;
    }
    else if (ability.shape == ABILITY_SHAPE_AREA)
    {
        float castDistance = toAimLength;
        if (castDistance > (float)ability.range)
        {
            castDistance = (float)ability.range;
        }
        geometry.centerX = casterX + geometry.dirX * castDistance;
        geometry.centerY = casterY + geometry.dirY * castDistance;
        geometry.radius = AREA_PLACEHOLDER_RADIUS;
    }

    return geometry;
}

bool IsPointInAimGeometry(const AimGeometry& geometry, float pointX, float pointY)
{
    if (geometry.shape == ABILITY_SHAPE_SELF || geometry.shape == ABILITY_SHAPE_AREA)
    {
        float circleDx = pointX - geometry.centerX;
        float circleDy = pointY - geometry.centerY;
        return (circleDx * circleDx + circleDy * circleDy) <= geometry.radius * geometry.radius;
    }

    float offsetX = pointX - geometry.originX;
    float offsetY = pointY - geometry.originY;
    float along = offsetX * geometry.dirX + offsetY * geometry.dirY;

    if (geometry.shape == ABILITY_SHAPE_LINE || geometry.shape == ABILITY_SHAPE_RANGED_SINGLE)
    {
        if (along < 0.0f || along > geometry.reach)
        {
            return false;
        }
        float across = offsetX * (-geometry.dirY) + offsetY * geometry.dirX;
        if (across < 0.0f)
        {
            across = -across;
        }
        return across <= geometry.width * 0.5f;
    }

    // Cone and adjacent wedge: within reach, and inside the half-angle of the aim direction
    float distance = std::sqrt(offsetX * offsetX + offsetY * offsetY);
    if (distance > geometry.reach)
    {
        return false;
    }
    if (distance < 0.001f)
    {
        return true;
    }
    float cosAngle = along / distance;
    return cosAngle >= std::cos(geometry.halfAngleDeg * DEG2RAD);
}

void DrawTargetingOverlay(float casterX, float casterY, float aimX, float aimY, const AbilityDef& ability, int tileSize)
{
    AimGeometry geometry = BuildAimGeometry(casterX, casterY, aimX, aimY, ability);
    float ts = (float)tileSize;
    Vector2 origin = { geometry.originX * ts, geometry.originY * ts };
    float aimAngleDeg = std::atan2(geometry.dirY, geometry.dirX) * RAD2DEG;
    Color fill = Fade(RED, 0.28f);
    Color edge = Fade(RED, 0.85f);

    if (geometry.shape == ABILITY_SHAPE_SELF)
    {
        Vector2 center = { geometry.centerX * ts, geometry.centerY * ts };
        DrawCircleV(center, geometry.radius * ts, fill);
        DrawCircleLinesV(center, geometry.radius * ts, edge);
    }
    else if (geometry.shape == ABILITY_SHAPE_AREA)
    {
        // Faint ring for how far the cast can land, then the blast itself under the cursor
        DrawCircleLinesV(origin, (float)ability.range * ts, Fade(RED, 0.2f));
        Vector2 center = { geometry.centerX * ts, geometry.centerY * ts };
        DrawCircleV(center, geometry.radius * ts, fill);
        DrawCircleLinesV(center, geometry.radius * ts, edge);
    }
    else if (geometry.shape == ABILITY_SHAPE_ADJACENT || geometry.shape == ABILITY_SHAPE_CONE)
    {
        float radiusPixels = geometry.reach * ts;
        float startAngle = aimAngleDeg - geometry.halfAngleDeg;
        float endAngle = aimAngleDeg + geometry.halfAngleDeg;
        DrawCircleSector(origin, radiusPixels, startAngle, endAngle, 24, fill);
        DrawCircleSectorLines(origin, radiusPixels, startAngle, endAngle, 24, edge);
    }
    else if (geometry.shape == ABILITY_SHAPE_LINE)
    {
        float lengthPixels = geometry.reach * ts;
        float widthPixels = geometry.width * ts;
        Rectangle body = { origin.x, origin.y, lengthPixels, widthPixels };
        Vector2 pivot = { 0.0f, widthPixels * 0.5f };
        DrawRectanglePro(body, pivot, aimAngleDeg, fill);

        float perpX = -geometry.dirY * widthPixels * 0.5f;
        float perpY = geometry.dirX * widthPixels * 0.5f;
        Vector2 end = { origin.x + geometry.dirX * lengthPixels, origin.y + geometry.dirY * lengthPixels };
        Vector2 sideAStart = { origin.x + perpX, origin.y + perpY };
        Vector2 sideAEnd = { end.x + perpX, end.y + perpY };
        Vector2 sideBStart = { origin.x - perpX, origin.y - perpY };
        Vector2 sideBEnd = { end.x - perpX, end.y - perpY };
        DrawLineEx(sideAStart, sideAEnd, 1.5f, edge);
        DrawLineEx(sideBStart, sideBEnd, 1.5f, edge);
        DrawLineEx(sideAEnd, sideBEnd, 1.5f, edge);
    }
    else if (geometry.shape == ABILITY_SHAPE_RANGED_SINGLE)
    {
        // Faint lane for what a shot can hit, bright center line and a dot where it runs out
        float lengthPixels = geometry.reach * ts;
        float widthPixels = geometry.width * ts;
        Rectangle lane = { origin.x, origin.y, lengthPixels, widthPixels };
        Vector2 pivot = { 0.0f, widthPixels * 0.5f };
        DrawRectanglePro(lane, pivot, aimAngleDeg, Fade(RED, 0.12f));

        Vector2 end = { origin.x + geometry.dirX * lengthPixels, origin.y + geometry.dirY * lengthPixels };
        DrawLineEx(origin, end, 2.0f, edge);
        DrawCircleV(end, 4.0f, edge);
    }
}