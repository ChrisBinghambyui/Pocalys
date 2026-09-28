#pragma once
#include "AbilityData.h"

// Free-form aim shape in tile space, not snapped to the grid. A tile's center is (x + 0.5, y + 0.5).
// The overlay draws this and the future ability execution reads it, so what you see is what gets hit.
struct AimGeometry
{
    AbilityShape shape;
    float originX;      // Where the shape starts, the caster's center
    float originY;
    float dirX;         // Unit vector toward the cursor
    float dirY;
    float reach;        // Length of a line, ray, cone, or adjacent wedge, in tiles
    float width;        // Line and ray thickness, in tiles
    float halfAngleDeg; // Cone and adjacent half-angle
    float centerX;      // Circle center. For AREA it is the cursor clamped to the ability's range.
    float centerY;
    float radius;       // Circle radius, in tiles
};

// casterX/casterY and aimX/aimY are tile-space centers (player.x + 0.5f, cursor world pixels / tileSize).
AimGeometry BuildAimGeometry(float casterX, float casterY, float aimX, float aimY, const AbilityDef& ability);

// True if a point (tile space) lies inside the shape. Use tile centers for enemies and items.
bool IsPointInAimGeometry(const AimGeometry& geometry, float pointX, float pointY);

// Draws the translucent aim shape. Call inside BeginMode2D/EndMode2D.
void DrawTargetingOverlay(float casterX, float casterY, float aimX, float aimY, const AbilityDef& ability, int tileSize);