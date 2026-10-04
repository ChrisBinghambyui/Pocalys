#pragma once
#include <string>
#include <vector>
#include <raylib.h>
#include "Player.h"
#include "AbilityData.h"
#include "WeaponData.h"

// What the avatar is doing, not which clip plays it. G_ANIM_ROWS in PlayerAnimation.cpp maps each
// category to a clip name, so a new rig or animation pack is a data change.
enum AnimCategory
{
    ANIM_IDLE,
    ANIM_MOVE,
    ANIM_MELEE_SLASH,
    ANIM_MELEE_PIERCE,
    ANIM_MELEE_CRUSH,
    ANIM_RANGED,
    ANIM_CAST,
    ANIM_INTERACT,
    ANIM_CATEGORY_COUNT
};

// Slashing, piercing, and crushing swings each get their own category.
AnimCategory GetMeleeAnimCategory(DamageType type);

// Derived from the ability's source, shape, and the equipped weapon, not hand-authored per ability.
AnimCategory GetAbilityAnimCategory(const Player& player, const AbilityDef& ability);

class PlayerAnimator
{
public:
    // Call once after the model loads. Loads every animation file in G_ANIM_FILES and reorders each
    // clip's bones to match the model (raylib pairs bones by index, not by name).
    void Load(const Model& model);
    void Unload();

    // Plays the clip for this category once, then returns to idle or move.
    // targetSeconds > 0 stretches or squeezes the clip to fit that time. 0 plays at the normal speed.
    void PlayOneShot(AnimCategory category, float targetSeconds);

    // Call once per frame before drawing the model.
    void Update(const Model& model, float deltaTime, float playerX, float playerY);

    // One line per category showing which clip it resolved to, for the debug overlay.
    std::vector<std::string> GetMappingLines() const;

private:
    std::vector<ModelAnimation*> _files;
    std::vector<int> _fileCounts;
    const ModelAnimation* _clipForCategory[ANIM_CATEGORY_COUNT] = { nullptr };
    const ModelAnimation* _current = nullptr;
    int _frame = 0;
    float _frameTimer = 0.0f;
    float _currentFps = 30.0f;
    bool _oneShotActive = false;
    float _lastX = 0.0f;
    float _lastY = 0.0f;

    const ModelAnimation* FindClip(const std::string& name) const;
    const ModelAnimation* PickLocomotionClip(bool moving) const;
    void StartClip(const ModelAnimation* clip, float fps);
};