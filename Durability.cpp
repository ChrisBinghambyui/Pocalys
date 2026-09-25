#include "Durability.h"
#include "MaterialData.h"
#include "raylib.h"

int GetConditionDamagePenalty(ConditionTier condition)
{
    if (condition == CONDITION_WORN)
    {
        return -1;
    }
    if (condition == CONDITION_DAMAGED)
    {
        return -2;
    }
    return 0;
}

int GetConditionArmorPenalty(ConditionTier condition)
{
    return GetConditionDamagePenalty(condition); // Same -1 Worn / -2 Damaged shape as weapons
}


int GetConditionRollPenalty(ConditionTier condition)
{
    if (condition == CONDITION_DAMAGED)
    {
        return 10;
    }
    return 0;
}

bool IsUsable(ConditionTier condition)
{
    return condition != CONDITION_BROKEN;
}

void RollDegradation(Item& item)
{
    if (item.materialTier < 0)
    {
        return; // Non-procedural items don't degrade through this system yet
    }
    if (item.condition == CONDITION_BROKEN)
    {
        return; // Already broken, nothing left to drop
    }

    int rate = G_MATERIAL_TIERS[item.materialTier].degradationRate;
    if (rate <= 0)
    {
        return; // Voidmetal/Chronesteel/Sundered tier, immune
    }

    int roll = GetRandomValue(1, 100);
    if (roll <= rate)
    {
        item.condition = (ConditionTier)(item.condition + 1);
    }
}