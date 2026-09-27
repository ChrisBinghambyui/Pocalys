#include "HotbarUI.h"
#include "AbilityHotbar.h"
#include "AbilityData.h"
#include "UIScale.h"
#include <raylib.h>
#include <string>

static const float HOVER_DESC_DELAY = 0.6f;

char GetHotbarKeyLabel(int slotIndex)
{
    int keyNumber = (slotIndex + 1) % 10;
    return (char)('0' + keyNumber);
}

static void DrawTooltip(int slotX, int slotY, int slotSize, const AbilityDef& ability, float hoverTime, float uiScale)
{
    int boxX = slotX + slotSize + (int)(14 * uiScale);
    int boxY = slotY;
    int boxW = (int)(380 * uiScale);
    bool showDescription = hoverTime >= HOVER_DESC_DELAY;
    int nameFont = (int)(24 * uiScale);
    int descFont = (int)(19 * uiScale);
    int boxH = showDescription ? (int)(130 * uiScale) : (int)(48 * uiScale);

    DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.9f));
    DrawRectangleLines(boxX, boxY, boxW, boxH, GOLD);
    DrawText(ability.name.c_str(), boxX + (int)(12 * uiScale), boxY + (int)(10 * uiScale), nameFont, GOLD);

    if (showDescription)
    {
        DrawText(ability.description.c_str(), boxX + (int)(12 * uiScale), boxY + (int)(48 * uiScale), descFont, LIGHTGRAY);
    }
}

void DrawHotbar(const Player& player)
{
    static int hoveredSlot = -1;
    static float hoverTimer = 0.0f;

    float uiScale = GetUIScale();
    int slotSize = (int)(64 * uiScale);
    int slotMargin = (int)(10 * uiScale);
    int startY = (int)(120 * uiScale);
    int drawX = (int)(18 * uiScale);

    Vector2 mouse = GetMousePosition();
    int currentHover = -1;

    for (size_t i = 0; i < player.hotbar.size(); i++)
    {
        int drawY = startY + (int)i * (slotSize + slotMargin);
        Rectangle slotRect = { (float)drawX, (float)drawY, (float)slotSize, (float)slotSize };

        std::string abilityId = player.hotbar[i];
        bool available = !abilityId.empty() && IsAbilityAvailable(player, abilityId);

        DrawRectangleRec(slotRect, Color{ 20, 16, 18, 220 });
        DrawRectangleLinesEx(slotRect, 2.0f * uiScale, available ? GOLD : DARKGRAY);

        std::string keyStr(1, GetHotbarKeyLabel((int)i));
        DrawText(keyStr.c_str(), drawX + (int)(6 * uiScale), drawY + (int)(4 * uiScale), (int)(18 * uiScale), LIGHTGRAY);

        if (!abilityId.empty())
        {
            const AbilityDef* ability = FindAbility(abilityId);
            if (ability != nullptr)
            {
                std::string iconStr(1, GetAbilityIcon(*ability));
                Color iconColor = available ? WHITE : GRAY;
                int iconFont = (int)(36 * uiScale);
                int iconWidth = MeasureText(iconStr.c_str(), iconFont);
                DrawText(iconStr.c_str(), drawX + (slotSize - iconWidth) / 2, drawY + (int)(18 * uiScale), iconFont, iconColor);
            }
        }

        if (CheckCollisionPointRec(mouse, slotRect))
        {
            currentHover = (int)i;
        }
    }

    if (currentHover == hoveredSlot && currentHover != -1)
    {
        hoverTimer += GetFrameTime();
    }
    else
    {
        hoveredSlot = currentHover;
        hoverTimer = 0.0f;
    }

    if (hoveredSlot != -1 && !player.hotbar[hoveredSlot].empty())
    {
        const AbilityDef* ability = FindAbility(player.hotbar[hoveredSlot]);
        if (ability != nullptr)
        {
            int slotY = startY + hoveredSlot * (slotSize + slotMargin);
            DrawTooltip(drawX, slotY, slotSize, *ability, hoverTimer, uiScale);
        }
    }
}