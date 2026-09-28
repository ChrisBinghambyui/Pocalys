#include "SkillScaling.h"

float GetSkillDamageMultiplier(int skillLevel)
{
    float multiplier = 1.0f + (skillLevel / 100.0f) * 0.5f;
    return multiplier;
}

int GetSkillCritChancePercent(int skillLevel)
{
    int chance = 5 + (skillLevel / 5);
    if (chance > 30)
    {
        chance = 30;
    }
    return chance;
}

int GetArmorSkillBonus(int skillLevel)
{
    if (skillLevel >= 100)
    {
        return 4;
    }
    if (skillLevel >= 75)
    {
        return 3;
    }
    if (skillLevel >= 50)
    {
        return 2;
    }
    if (skillLevel >= 25)
    {
        return 1;
    }
    return 0;
}

float GetSkillSpeedMultiplier(int skillLevel)
{
    float multiplier = 1.0f + (skillLevel / 100.0f) * 0.3f;
    return multiplier;
}