#pragma once
#include "AbilityData.h"
#include <vector>

struct TilePos { int x; int y; };

// Tiles an aimed ability would affect, given the caster's tile, the hovered tile, and the ability's
// shape/range. World tile coordinates, not screen pixels.
std::vector<TilePos> GetTargetedTiles(int casterX, int casterY, int hoverX, int hoverY, const AbilityDef& ability);

// Draws a translucent highlight over each targeted tile. Call inside BeginMode2D/EndMode2D.
void DrawTargetingOverlay(int casterX, int casterY, int hoverX, int hoverY, const AbilityDef& ability, int tileSize);