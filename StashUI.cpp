#include "StashUI.h"
#include "StashData.h"
#include "UIScale.h"
#include <raylib.h>
#include <string>

// Defined in Rogue-Project.cpp for now. Moves to an item naming header when that extraction happens.
std::string GetGroundItemName(const Item& item);

static std::string GetConditionText(ConditionTier condition)
{
    if (condition == CONDITION_PRISTINE)
    {
        return "Pristine";
    }
    if (condition == CONDITION_GOOD)
    {
        return "Good";
    }
    if (condition == CONDITION_WORN)
    {
        return "Worn";
    }
    if (condition == CONDITION_DAMAGED)
    {
        return "Damaged";
    }
    return "Broken";
}

static int CountPacked(const StashLoadoutState& state)
{
    int count = 0;
    for (size_t i = 0; i < state.packed.size(); i++)
    {
        if (state.packed[i])
        {
            count++;
        }
    }
    return count;
}

static void TogglePacked(StashLoadoutState& state, int index)
{
    if (index < 0 || index >= (int)state.packed.size())
    {
        return;
    }
    if (state.packed[index])
    {
        state.packed[index] = false;
        return;
    }
    if (CountPacked(state) < STASH_PACK_LIMIT)
    {
        state.packed[index] = true;
    }
}

StashUIAction UpdateAndDrawStashLoadout(StashLoadoutState& state)
{
    StashUIAction result = STASH_UI_NONE;
    int itemCount = (int)G_STASH.items.size();

    if ((int)state.packed.size() != itemCount)
    {
        state.packed.assign(itemCount, false);
        state.selected = 0;
    }

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        if (state.selected > 0)
        {
            state.selected--;
        }
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        if (state.selected < itemCount - 1)
        {
            state.selected++;
        }
    }
    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE))
    {
        TogglePacked(state, state.selected);
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        result = STASH_UI_BEGIN;
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        result = STASH_UI_BACK;
    }

    float uiScale = GetUIScale();
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int listX = (int)(80 * uiScale);
    int listTop = (int)(150 * uiScale);
    int rowHeight = (int)(36 * uiScale);
    int footerHeight = (int)(90 * uiScale);
    int rowFont = (int)(24 * uiScale);

    int visibleRows = (screenH - listTop - footerHeight) / rowHeight;
    if (visibleRows < 1)
    {
        visibleRows = 1;
    }
    int firstVisible = 0;
    if (state.selected >= visibleRows)
    {
        firstVisible = state.selected - visibleRows + 1;
    }

    Vector2 mouse = GetMousePosition();
    for (int r = 0; r < visibleRows; r++)
    {
        int index = firstVisible + r;
        if (index >= itemCount)
        {
            break;
        }
        Rectangle rowRect = { (float)listX, (float)(listTop + r * rowHeight), (float)(screenW - listX * 2), (float)rowHeight };
        if (CheckCollisionPointRec(mouse, rowRect))
        {
            state.selected = index;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                TogglePacked(state, index);
            }
        }
    }

    BeginDrawing();
    ClearBackground(Color{ 18, 12, 14, 255 });

    int titleFont = (int)(32 * uiScale);
    const char* titleText = "PACK YOUR BAG";
    DrawText(titleText, screenW / 2 - MeasureText(titleText, titleFont) / 2, (int)(30 * uiScale), titleFont, GOLD);

    int subFont = (int)(22 * uiScale);
    std::string subText = "Bring up to " + std::to_string(STASH_PACK_LIMIT) + " stacks from the stash. What you leave stays banked.";
    DrawText(subText.c_str(), screenW / 2 - MeasureText(subText.c_str(), subFont) / 2, (int)(80 * uiScale), subFont, LIGHTGRAY);

    std::string countText = "Packed: " + std::to_string(CountPacked(state)) + " / " + std::to_string(STASH_PACK_LIMIT);
    DrawText(countText.c_str(), listX, (int)(115 * uiScale), subFont, SKYBLUE);

    for (int r = 0; r < visibleRows; r++)
    {
        int index = firstVisible + r;
        if (index >= itemCount)
        {
            break;
        }
        const Item& item = G_STASH.items[index];

        std::string line = "[ ] ";
        if (state.packed[index])
        {
            line = "[x] ";
        }
        line += GetGroundItemName(item);
        if (item.quantity > 1)
        {
            line += " x" + std::to_string(item.quantity);
        }
        line += "  (" + GetConditionText(item.condition) + ")";

        Color lineColor = WHITE;
        if (state.packed[index])
        {
            lineColor = GREEN;
        }
        if (index == state.selected)
        {
            lineColor = YELLOW;
            DrawText(">", listX - (int)(28 * uiScale), listTop + r * rowHeight, rowFont, YELLOW);
        }
        DrawText(line.c_str(), listX, listTop + r * rowHeight, rowFont, lineColor);
    }

    const char* hintText = "[W/S] Move  |  [E / Space / Click] Pack or unpack  |  [Enter] Begin  |  [Esc] Back";
    int hintFont = (int)(22 * uiScale);
    DrawText(hintText, screenW / 2 - MeasureText(hintText, hintFont) / 2, screenH - (int)(50 * uiScale), hintFont, GOLD);
    EndDrawing();

    return result;
}