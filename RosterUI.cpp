#include "RosterUI.h"
#include "RosterData.h"
#include "FeatData.h"
#include "RaceData.h"
#include "UIScale.h"
#include <raylib.h>
#include <string>

// Defined in Rogue-Project.cpp for now. Moves to an item naming header when that extraction happens.
std::string GetGroundItemName(const Item& item);

RosterUIAction UpdateAndDrawRoster(RosterViewState& state, int& outPickedIndex)
{
    RosterUIAction result = ROSTER_UI_NONE;
    int count = (int)G_ROSTER.size();
    if (count == 0)
    {
        return ROSTER_UI_BACK;
    }
    if (state.selected >= count)
    {
        state.selected = count - 1;
    }
    if (state.selected < 0)
    {
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
        if (state.selected < count - 1)
        {
            state.selected++;
        }
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        result = ROSTER_UI_BEGIN;
        outPickedIndex = state.selected;
    }
    if (IsKeyPressed(KEY_ESCAPE))
    {
        result = ROSTER_UI_BACK;
    }

    float uiScale = GetUIScale();
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    int listX = (int)(60 * uiScale);
    int listW = (int)(screenW * 0.38f);
    int listTop = (int)(140 * uiScale);
    int rowHeight = (int)(40 * uiScale);
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
        if (index >= count)
        {
            break;
        }
        Rectangle rowRect = { (float)listX, (float)(listTop + r * rowHeight), (float)listW, (float)rowHeight };
        if (CheckCollisionPointRec(mouse, rowRect))
        {
            state.selected = index;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                result = ROSTER_UI_BEGIN;
                outPickedIndex = index;
            }
        }
    }

    BeginDrawing();
    ClearBackground(Color{ 18, 12, 14, 255 });

    int titleFont = (int)(32 * uiScale);
    const char* titleText = "VETERANS";
    DrawText(titleText, screenW / 2 - MeasureText(titleText, titleFont) / 2, (int)(30 * uiScale), titleFont, GOLD);
    int subFont = (int)(22 * uiScale);
    const char* subText = "Characters who made it out alive. Pick one to lead the next expedition.";
    DrawText(subText, screenW / 2 - MeasureText(subText, subFont) / 2, (int)(80 * uiScale), subFont, LIGHTGRAY);

    for (int r = 0; r < visibleRows; r++)
    {
        int index = firstVisible + r;
        if (index >= count)
        {
            break;
        }
        const Player& veteran = G_ROSTER[index];
        std::string line = veteran.name + "  (Lv " + std::to_string(veteran.level) + " " + veteran.className + ")";
        Color lineColor = WHITE;
        if (index == state.selected)
        {
            lineColor = YELLOW;
            DrawText(">", listX - (int)(28 * uiScale), listTop + r * rowHeight, rowFont, YELLOW);
        }
        DrawText(line.c_str(), listX, listTop + r * rowHeight, rowFont, lineColor);
    }

    // Detail panel for the highlighted veteran
    const Player& chosen = G_ROSTER[state.selected];
    int panelX = listX + listW + (int)(40 * uiScale);
    int panelY = listTop;
    int panelW = screenW - panelX - (int)(60 * uiScale);
    int panelH = screenH - panelY - footerHeight;
    DrawRectangle(panelX, panelY, panelW, panelH, Color{ 28, 22, 24, 255 });
    DrawRectangleLinesEx(Rectangle{ (float)panelX, (float)panelY, (float)panelW, (float)panelH }, 1.5f, GOLD);

    int pad = (int)(18 * uiScale);
    int detailY = panelY + pad;
    int nameFont = (int)(30 * uiScale);
    int lineFont = (int)(22 * uiScale);
    int smallFont = (int)(19 * uiScale);
    int panelBottom = panelY + panelH - pad;

    DrawText(chosen.name.c_str(), panelX + pad, detailY, nameFont, GOLD);
    detailY += (int)(nameFont * 1.4f);

    std::string raceName = "Unknown";
    const RaceData* race = FindRace(chosen.raceId);
    if (race != nullptr)
    {
        raceName = race->name;
    }
    std::string identity = raceName + "  |  " + chosen.className + "  |  Level " + std::to_string(chosen.level) + "  |  " + chosen.birthsign;
    DrawText(identity.c_str(), panelX + pad, detailY, lineFont, VIOLET);
    detailY += (int)(lineFont * 1.5f);

    std::string vitals = "HP " + std::to_string(chosen.maxHp) + "   SP " + std::to_string(chosen.maxStamina) + "   MP " + std::to_string(chosen.maxMana);
    DrawText(vitals.c_str(), panelX + pad, detailY, lineFont, GREEN);
    detailY += (int)(lineFont * 1.5f);

    std::string attributes = "STR " + std::to_string(chosen.str) + "  END " + std::to_string(chosen.end) + "  AGI " + std::to_string(chosen.agi) + "  INT " + std::to_string(chosen.intel);
    attributes += "  WIL " + std::to_string(chosen.wil) + "  PER " + std::to_string(chosen.per) + "  LCK " + std::to_string(chosen.lck);
    DrawText(attributes.c_str(), panelX + pad, detailY, smallFont, SKYBLUE);
    detailY += (int)(smallFont * 1.8f);

    DrawText("FEATS / BOONS:", panelX + pad, detailY, lineFont, GOLD);
    detailY += (int)(lineFont * 1.3f);
    if (chosen.activeFeats.empty() && chosen.activeBoons.empty())
    {
        DrawText("None yet.", panelX + pad + (int)(10 * uiScale), detailY, smallFont, LIGHTGRAY);
        detailY += (int)(smallFont * 1.4f);
    }
    for (size_t i = 0; i < chosen.activeFeats.size() && detailY < panelBottom; i++)
    {
        const Feat* feat = FindFeat(chosen.activeFeats[i]);
        if (feat != nullptr)
        {
            DrawText(feat->name.c_str(), panelX + pad + (int)(10 * uiScale), detailY, smallFont, WHITE);
            detailY += (int)(smallFont * 1.4f);
        }
    }
    for (size_t i = 0; i < chosen.activeBoons.size() && detailY < panelBottom; i++)
    {
        const Feat* boon = FindFeat(chosen.activeBoons[i]);
        if (boon != nullptr)
        {
            DrawText(boon->name.c_str(), panelX + pad + (int)(10 * uiScale), detailY, smallFont, ORANGE);
            detailY += (int)(smallFont * 1.4f);
        }
    }

    detailY += (int)(smallFont * 0.6f);
    if (detailY < panelBottom)
    {
        DrawText("EQUIPPED:", panelX + pad, detailY, lineFont, GOLD);
        detailY += (int)(lineFont * 1.3f);
    }
    for (int s = 0; s < SLOT_SINGLE_COUNT && detailY < panelBottom; s++)
    {
        if (!chosen.equippedSlots[s].IsEmpty())
        {
            std::string itemName = GetGroundItemName(chosen.equippedSlots[s]);
            DrawText(itemName.c_str(), panelX + pad + (int)(10 * uiScale), detailY, smallFont, WHITE);
            detailY += (int)(smallFont * 1.4f);
        }
    }

    const char* hintText = "[W/S] Move  |  [Enter / Click] Lead the expedition  |  [Esc] Back";
    int hintFont = (int)(22 * uiScale);
    DrawText(hintText, screenW / 2 - MeasureText(hintText, hintFont) / 2, screenH - (int)(50 * uiScale), hintFont, GOLD);
    EndDrawing();

    return result;
}