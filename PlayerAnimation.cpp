#include "PlayerAnimation.h"
#include <string>
#include <vector>

static const float DEFAULT_ANIM_FPS = 30.0f;
static const float MIN_ONE_SHOT_FPS = 15.0f;
static const float MAX_ONE_SHOT_FPS = 90.0f;
static const float MOVING_DISTANCE_SQUARED = 0.00001f;

// Every file listed here that exists on disk gets loaded. Missing files are skipped, so adding an
// animation pack later is one line, and not owning a pack costs nothing.
static const std::vector<std::string> G_ANIM_FILES = {
    "assets/animations/gltf/Rig_Medium/Rig_Medium_General.glb",
    "assets/animations/gltf/Rig_Medium/Rig_Medium_MovementBasic.glb",
    "assets/animations/gltf/Rig_Medium/Rig_Medium_CombatMelee.glb",
    "assets/animations/gltf/Rig_Medium/Rig_Medium_CombatRanged.glb",
    "assets/animations/gltf/Rig_Medium/Rig_Medium_Special.glb"
};

struct AnimRow
{
    AnimCategory category;
    std::string clipName;
};

// For each category the first row whose clip exists wins. Rows are in priority order.
// Throw and Use_Item are stand-ins that exist in the General file, so attacks and spells show
// something until the melee, ranged, and magic clips are loaded. The combat clip names are best
// guesses at KayKit's naming, the debug overlay shows what actually resolved.
static const std::vector<AnimRow> G_ANIM_ROWS = {
    { ANIM_IDLE, "Idle_A" },
    { ANIM_IDLE, "Idle_B" },
    { ANIM_MOVE, "Running_A" },
    { ANIM_MOVE, "Walking_A" },
    { ANIM_MELEE_SLASH, "Melee_1H_Attack_Slice_Horizontal" },
    { ANIM_MELEE_SLASH, "Melee_1H_Attack_Slice_Diagonal" },
    { ANIM_MELEE_SLASH, "Throw" },
    { ANIM_MELEE_PIERCE, "Melee_1H_Attack_Stab" },
    { ANIM_MELEE_PIERCE, "Throw" },
    { ANIM_MELEE_CRUSH, "Melee_1H_Attack_Chop" },
    { ANIM_MELEE_CRUSH, "Throw" },
    { ANIM_RANGED, "Ranged_1H_Shoot" },
    { ANIM_RANGED, "Ranged_Bow_Release" },
    { ANIM_RANGED, "Throw" },
    { ANIM_CAST, "Ranged_Magic_Shoot" },
    { ANIM_CAST, "Ranged_Magic_Spellcasting" },
    { ANIM_CAST, "Use_Item" },
    { ANIM_INTERACT, "Interact" }
};

static const char* G_ANIM_CATEGORY_LABELS[ANIM_CATEGORY_COUNT] = {
    "idle", "move", "slash", "pierce", "crush", "ranged", "cast", "interact"
};

// Raylib pairs animation bones to model bones by index. This reorders the animation into the
// model's bone order by name. False (and the animation is left untouched) if any model bone is missing.
static bool RemapAnimationToModel(const Model& model, ModelAnimation& anim)
{
    if (model.boneCount != anim.boneCount)
    {
        return false;
    }

    int boneCount = model.boneCount;
    std::vector<int> animIndexForModelBone(boneCount, -1);
    std::vector<int> modelIndexForAnimBone(boneCount, -1);
    for (int m = 0; m < boneCount; m++)
    {
        std::string modelName = model.bones[m].name;
        for (int a = 0; a < boneCount; a++)
        {
            std::string animName = anim.bones[a].name;
            if (modelName == animName)
            {
                animIndexForModelBone[m] = a;
                modelIndexForAnimBone[a] = m;
                break;
            }
        }
        if (animIndexForModelBone[m] < 0)
        {
            return false;
        }
    }

    for (int f = 0; f < anim.frameCount; f++)
    {
        std::vector<Transform> reordered(boneCount);
        for (int m = 0; m < boneCount; m++)
        {
            reordered[m] = anim.framePoses[f][animIndexForModelBone[m]];
        }
        for (int m = 0; m < boneCount; m++)
        {
            anim.framePoses[f][m] = reordered[m];
        }
    }

    std::vector<BoneInfo> reorderedBones(boneCount);
    for (int m = 0; m < boneCount; m++)
    {
        reorderedBones[m] = anim.bones[animIndexForModelBone[m]];
        if (reorderedBones[m].parent >= 0)
        {
            reorderedBones[m].parent = modelIndexForAnimBone[reorderedBones[m].parent];
        }
    }
    for (int m = 0; m < boneCount; m++)
    {
        anim.bones[m] = reorderedBones[m];
    }
    return true;
}

// ---------- Category helpers ----------

AnimCategory GetMeleeAnimCategory(DamageType type)
{
    if (type == DAMAGE_SLASHING)
    {
        return ANIM_MELEE_SLASH;
    }
    if (type == DAMAGE_PIERCING)
    {
        return ANIM_MELEE_PIERCE;
    }
    return ANIM_MELEE_CRUSH;
}

AnimCategory GetAbilityAnimCategory(const Player& player, const AbilityDef& ability)
{
    if (ability.source == ABILITY_SOURCE_SPELL)
    {
        return ANIM_CAST;
    }

    const Item& weapon = player.equippedSlots[SLOT_MAIN_HAND];
    if (weapon.weaponTypeId >= 0 && weapon.weaponTypeId < (int)G_WEAPON_TYPES.size())
    {
        if (G_WEAPON_TYPES[weapon.weaponTypeId].category == WEAPON_RANGED)
        {
            return ANIM_RANGED;
        }
    }

    // Melee abilities: a line is a thrust, a cone is a sweep, anything else is an overhead blow
    if (ability.shape == ABILITY_SHAPE_LINE)
    {
        return ANIM_MELEE_PIERCE;
    }
    if (ability.shape == ABILITY_SHAPE_CONE)
    {
        return ANIM_MELEE_SLASH;
    }
    return ANIM_MELEE_CRUSH;
}

// ---------- PlayerAnimator ----------

void PlayerAnimator::Load(const Model& model)
{
    Unload();

    for (size_t i = 0; i < G_ANIM_FILES.size(); i++)
    {
        if (!FileExists(G_ANIM_FILES[i].c_str()))
        {
            continue;
        }

        int count = 0;
        ModelAnimation* clips = LoadModelAnimations(G_ANIM_FILES[i].c_str(), &count);
        if (clips == nullptr || count <= 0)
        {
            continue;
        }

        for (int c = 0; c < count; c++)
        {
            RemapAnimationToModel(model, clips[c]);
        }
        _files.push_back(clips);
        _fileCounts.push_back(count);
    }

    for (int c = 0; c < ANIM_CATEGORY_COUNT; c++)
    {
        _clipForCategory[c] = nullptr;
    }
    for (size_t r = 0; r < G_ANIM_ROWS.size(); r++)
    {
        int category = (int)G_ANIM_ROWS[r].category;
        if (_clipForCategory[category] != nullptr)
        {
            continue;
        }
        _clipForCategory[category] = FindClip(G_ANIM_ROWS[r].clipName);
    }

    _oneShotActive = false;
    StartClip(_clipForCategory[ANIM_IDLE], DEFAULT_ANIM_FPS);
}

void PlayerAnimator::Unload()
{
    for (size_t i = 0; i < _files.size(); i++)
    {
        UnloadModelAnimations(_files[i], _fileCounts[i]);
    }
    _files.clear();
    _fileCounts.clear();
    for (int c = 0; c < ANIM_CATEGORY_COUNT; c++)
    {
        _clipForCategory[c] = nullptr;
    }
    _current = nullptr;
    _oneShotActive = false;
}

const ModelAnimation* PlayerAnimator::FindClip(const std::string& name) const
{
    for (size_t f = 0; f < _files.size(); f++)
    {
        for (int c = 0; c < _fileCounts[f]; c++)
        {
            std::string clipName = _files[f][c].name;
            if (clipName == name)
            {
                return &_files[f][c];
            }
        }
    }
    return nullptr;
}

const ModelAnimation* PlayerAnimator::PickLocomotionClip(bool moving) const
{
    if (moving && _clipForCategory[ANIM_MOVE] != nullptr)
    {
        return _clipForCategory[ANIM_MOVE];
    }
    return _clipForCategory[ANIM_IDLE];
}

void PlayerAnimator::StartClip(const ModelAnimation* clip, float fps)
{
    _current = clip;
    _frame = 0;
    _frameTimer = 0.0f;
    _currentFps = fps;
}

void PlayerAnimator::PlayOneShot(AnimCategory category, float targetSeconds)
{
    const ModelAnimation* clip = _clipForCategory[category];
    if (clip == nullptr || clip->frameCount <= 0)
    {
        return;
    }

    float fps = DEFAULT_ANIM_FPS;
    if (targetSeconds > 0.0f)
    {
        fps = (float)clip->frameCount / targetSeconds;
        if (fps < MIN_ONE_SHOT_FPS)
        {
            fps = MIN_ONE_SHOT_FPS;
        }
        if (fps > MAX_ONE_SHOT_FPS)
        {
            fps = MAX_ONE_SHOT_FPS;
        }
    }

    StartClip(clip, fps);
    _oneShotActive = true;
}

void PlayerAnimator::Update(const Model& model, float deltaTime, float playerX, float playerY)
{
    float movedX = playerX - _lastX;
    float movedY = playerY - _lastY;
    _lastX = playerX;
    _lastY = playerY;
    bool moving = (movedX * movedX + movedY * movedY) > MOVING_DISTANCE_SQUARED;

    if (!_oneShotActive)
    {
        const ModelAnimation* wanted = PickLocomotionClip(moving);
        if (wanted != _current)
        {
            StartClip(wanted, DEFAULT_ANIM_FPS);
        }
    }

    if (_current == nullptr || _current->frameCount <= 0)
    {
        return;
    }

    _frameTimer += deltaTime;
    float frameStep = 1.0f / _currentFps;
    while (_frameTimer >= frameStep)
    {
        _frameTimer -= frameStep;
        _frame++;
    }

    if (_frame >= _current->frameCount)
    {
        if (_oneShotActive)
        {
            _oneShotActive = false;
            StartClip(PickLocomotionClip(moving), DEFAULT_ANIM_FPS);
            if (_current == nullptr || _current->frameCount <= 0)
            {
                return;
            }
        }
        else
        {
            _frame = _frame % _current->frameCount;
        }
    }

    UpdateModelAnimation(model, *_current, _frame);
}

std::vector<std::string> PlayerAnimator::GetMappingLines() const
{
    std::vector<std::string> lines;
    for (int c = 0; c < ANIM_CATEGORY_COUNT; c++)
    {
        std::string line = std::string(G_ANIM_CATEGORY_LABELS[c]) + ": ";
        if (_clipForCategory[c] == nullptr)
        {
            line += "(none)";
        }
        else
        {
            line += _clipForCategory[c]->name;
        }
        lines.push_back(line);
    }
    return lines;
}