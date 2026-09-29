#include "WeaponVisual.h"
#include "Combat.h"
#include <raylib.h>
#include <cmath>

// All lengths are tile units.
static const float WEAPON_LENGTH_LIGHT = 0.55f;
static const float WEAPON_LENGTH_MEDIUM = 0.8f;
static const float WEAPON_LENGTH_HEAVY = 1.05f;
static const float WEAPON_REACH_BONUS = 0.25f; // Spears and halberds draw longer
static const float FIST_LENGTH = 0.25f;

static const float BODY_OFFSET = 0.3f;       // How far from the player's center the weapon is held
static const float IDLE_HAND_ANGLE = 1.1f;   // Radians off the facing direction, the weapon rests at the hip
static const float IDLE_BLADE_ANGLE = 0.3f;  // Radians off the facing direction, canted slightly forward
static const float SLASH_HALF_SWEEP = 1.2f;  // Radians either side of the aim, roughly 70 degrees
static const float THRUST_LUNGE = 0.55f;     // How far the whole weapon shoves forward
static const float BASH_BUMP = 0.4f;
static const float BASH_HOLD_FORWARD = 0.15f; // The crosswise weapon is held a little further out than the resting hand

static const int TRAIL_STEPS = 3;
static const float TRAIL_SPACING = 0.07f;    // Swing progress between trail ghosts
static const float MIN_SWING_DURATION = 0.08f;

struct WeaponSegment
{
    Vector2 hand;
    Vector2 tip;
};

static Vector2 PointAtAngle(Vector2 from, float angle, float distance)
{
    Vector2 result = { from.x + std::cos(angle) * distance, from.y + std::sin(angle) * distance };
    return result;
}

static float GetVisualWeaponLength(const Player& player)
{
    const WeaponType* weapon = GetMeleeWeaponType(player);
    if (weapon == nullptr)
    {
        return FIST_LENGTH;
    }

    float length = WEAPON_LENGTH_MEDIUM;
    if (weapon->category == WEAPON_LIGHT)
    {
        length = WEAPON_LENGTH_LIGHT;
    }
    else if (weapon->category == WEAPON_HEAVY)
    {
        length = WEAPON_LENGTH_HEAVY;
    }
    if (weapon->reach)
    {
        length += WEAPON_REACH_BONUS;
    }
    return length;
}

static Color GetSwingColor(DamageType type)
{
    if (type == DAMAGE_PIERCING)
    {
        return SKYBLUE;
    }
    if (type == DAMAGE_CRUSHING)
    {
        return ORANGE;
    }
    return RAYWHITE;
}

static WeaponSegment GetIdleSegment(Vector2 center, float aimAngle, float length)
{
    WeaponSegment segment;
    segment.hand = PointAtAngle(center, aimAngle + IDLE_HAND_ANGLE, BODY_OFFSET);
    segment.tip = PointAtAngle(segment.hand, aimAngle + IDLE_BLADE_ANGLE, length);
    return segment;
}

// Slash: the weapon sweeps an arc across the aim. Thrust: it stays pointed at the aim and lunges out and back.
// Bash: it turns crosswise and bumps forward, like a haft or pommel shoved into someone.
static WeaponSegment GetSwingSegment(const WeaponSwingState& swing, float progress, Vector2 center, float length)
{
    float eased = 1.0f - (1.0f - progress) * (1.0f - progress); // Fast start, soft finish
    WeaponSegment segment;

    if (swing.type == DAMAGE_SLASHING)
    {
        float sweep = SLASH_HALF_SWEEP;
        if (swing.flip)
        {
            sweep = -sweep;
        }
        Vector2 pivot = PointAtAngle(center, swing.aimAngle, BODY_OFFSET);
        float bladeAngle = swing.aimAngle - sweep + (2.0f * sweep * eased);
        segment.hand = pivot;
        segment.tip = PointAtAngle(pivot, bladeAngle, length);
    }
    else if (swing.type == DAMAGE_PIERCING)
    {
        float lunge = std::sin(progress * PI) * THRUST_LUNGE;
        segment.hand = PointAtAngle(center, swing.aimAngle, BODY_OFFSET + lunge);
        segment.tip = PointAtAngle(segment.hand, swing.aimAngle, length);
    }
    else
    {
        float bump = std::sin(progress * PI) * BASH_BUMP;
        Vector2 middle = PointAtAngle(center, swing.aimAngle, BODY_OFFSET + BASH_HOLD_FORWARD + bump);
        float crossAngle = swing.aimAngle + PI * 0.5f;
        segment.hand = PointAtAngle(middle, crossAngle, -length * 0.5f);
        segment.tip = PointAtAngle(middle, crossAngle, length * 0.5f);
    }

    return segment;
}

static void DrawSegment(const WeaponSegment& segment, float tileSize, float thickness, Color color)
{
    Vector2 handPixels = { segment.hand.x * tileSize, segment.hand.y * tileSize };
    Vector2 tipPixels = { segment.tip.x * tileSize, segment.tip.y * tileSize };
    DrawLineEx(handPixels, tipPixels, thickness, color);
}

void StartWeaponSwing(WeaponSwingState& swing, const Player& player, DamageType type, float duration)
{
    DamageType visualType = type;
    if (GetMeleeWeaponType(player) == nullptr)
    {
        visualType = DAMAGE_PIERCING; // A punch reads as a jab
    }

    if (visualType == DAMAGE_SLASHING)
    {
        swing.flip = !swing.flip;
    }

    if (duration < MIN_SWING_DURATION)
    {
        duration = MIN_SWING_DURATION;
    }

    swing.active = true;
    swing.type = visualType;
    swing.timer = 0.0f;
    swing.duration = duration;
    swing.aimAngle = std::atan2(player.facingY, player.facingX);
}

void UpdateWeaponSwing(WeaponSwingState& swing, float deltaTime)
{
    if (!swing.active)
    {
        return;
    }
    swing.timer += deltaTime;
    if (swing.timer >= swing.duration)
    {
        swing.active = false;
    }
}

void DrawWeaponVisual(const WeaponSwingState& swing, const Player& player, int tileSize)
{
    float ts = (float)tileSize;
    Vector2 center = { player.x + 0.5f, player.y + 0.5f };
    float length = GetVisualWeaponLength(player);

    if (!swing.active)
    {
        float idleAngle = std::atan2(player.facingY, player.facingX);
        WeaponSegment idle = GetIdleSegment(center, idleAngle, length);
        DrawSegment(idle, ts, 2.0f, GRAY);
        return;
    }

    float progress = swing.timer / swing.duration;
    if (progress > 1.0f)
    {
        progress = 1.0f;
    }
    Color color = GetSwingColor(swing.type);

    // Faint ghosts of where the weapon just was, drawn first so the live line sits on top
    for (int i = TRAIL_STEPS; i >= 1; i--)
    {
        float trailProgress = progress - (float)i * TRAIL_SPACING;
        if (trailProgress < 0.0f)
        {
            continue;
        }
        float alpha = 0.5f - (float)i * 0.12f;
        WeaponSegment ghost = GetSwingSegment(swing, trailProgress, center, length);
        DrawSegment(ghost, ts, 2.0f, Fade(color, alpha));
    }

    WeaponSegment live = GetSwingSegment(swing, progress, center, length);
    DrawSegment(live, ts, 3.0f, color);
}