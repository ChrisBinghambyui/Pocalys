#pragma once
#include "Player.h"
#include "WeaponData.h"

// Placeholder wire-line weapon drawn beside the player. Presentation only, it never touches combat.
struct WeaponSwingState
{
    bool active = false;
    DamageType type = DAMAGE_SLASHING;
    float timer = 0.0f;    // Seconds since the swing started
    float duration = 0.2f;
    float aimAngle = 0.0f; // Radians. Facing captured at the click so the visual matches what was hit.
    bool flip = false;     // Slashes alternate direction so back-to-back swings read as a rhythm
};

// Call when a swing is thrown. Bare fists (or a broken or ranged weapon) always jab, whatever type is passed.
void StartWeaponSwing(WeaponSwingState& swing, const Player& player, DamageType type, float duration);

void UpdateWeaponSwing(WeaponSwingState& swing, float deltaTime);

// Call inside BeginMode2D/EndMode2D, after the player glyph. Draws the resting weapon when no swing is active.
void DrawWeaponVisual(const WeaponSwingState& swing, const Player& player, int tileSize);