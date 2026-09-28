#include "MenuHubUI.h"
#include "AbilityHotbar.h"
#include "AbilityData.h"
#include "FeatData.h"
#include "RaceData.h"
#include "SkillData.h"
#include "UIScale.h"
#include <raylib.h>
#include <string>
#include <vector>

static const char* TAB_NAMES[MENU_TAB_COUNT] = { "INVENTORY", "ABILITIES", "MAGIC", "CHARACTER" };

int GetMenuTabBarHeight()
{
    return (int)(60 * GetUIScale());
}

int GetMenuFooterHeight()
{
    return (int)(70 * GetUIScale());
}

int GetMenuTabAtPoint(int panelX, int panelY, int panelW, int pointX, int pointY)
{
    int tabBarHeight = GetMenuTabBarHeight();
    int tabWidth = panelW / MENU_TAB_COUNT;
    if (pointY < panelY || pointY >= panelY + tabBarHeight)
    {
        return -1;
    }
    if (pointX < panelX)
    {
        return -1;
    }
    int index = (pointX - panelX) / tabWidth;
    if (index >= MENU_TAB_COUNT)
    {
        return -1;
    }
    return index;
}

void DrawMenuTabBar(int panelX, int panelY, int panelW, MenuTab currentTab)
{
    float uiScale = GetUIScale();
    int tabBarHeight = GetMenuTabBarHeight();
    int tabWidth = panelW / MENU_TAB_COUNT;
    int fontSize = (int)(26 * uiScale);

    for (int i = 0; i < MENU_TAB_COUNT; i++)
    {
        int tabX = panelX + i * tabWidth;
        bool isActive = (i == (int)currentTab);
        Color bgColor = isActive ? Color{ 45, 32, 28, 255 } : Color{ 20, 16, 18, 255 };
        Color textColor = isActive ? GOLD : GRAY;

        DrawRectangle(tabX, panelY, tabWidth, tabBarHeight, bgColor);
        DrawRectangleLines(tabX, panelY, tabWidth, tabBarHeight, isActive ? GOLD : DARKGRAY);

        int textWidth = MeasureText(TAB_NAMES[i], fontSize);
        DrawText(TAB_NAMES[i], tabX + (tabWidth - textWidth) / 2, panelY + (tabBarHeight - fontSize) / 2, fontSize, textColor);
    }
}

void DrawAbilitiesTab(int panelX, int panelY, int panelW, int panelH, const Player& player)
{
    float uiScale = GetUIScale();
    int nameFont = (int)(26 * uiScale);
    int descFont = (int)(20 * uiScale);
    int rowHeight = (int)(70 * uiScale);
    int iconFont = (int)(28 * uiScale);

    std::vector<std::string> pool = BuildAbilityPool(player);
    int drawY = panelY + GetMenuTabBarHeight() + (int)(24 * uiScale);
    int contentBottom = panelY + panelH - GetMenuFooterHeight();

    if (pool.empty())
    {
        DrawText("No weapon abilities available with your current loadout.", panelX + (int)(24 * uiScale), drawY, nameFont, LIGHTGRAY);
        return;
    }

    for (size_t i = 0; i < pool.size(); i++)
    {
        if (drawY > contentBottom)
        {
            break;
        }

        const AbilityDef* ability = FindAbility(pool[i]);
        if (ability == nullptr)
        {
            continue;
        }

        std::string hotbarSlotText = "unassigned";
        for (size_t slot = 0; slot < player.hotbar.size(); slot++)
        {
            if (player.hotbar[slot] == pool[i])
            {
                int keyNumber = (int)((slot + 1) % 10);
                hotbarSlotText = "key " + std::to_string(keyNumber);
                break;
            }
        }

        std::string iconStr(1, GetAbilityIcon(*ability));
        DrawText(iconStr.c_str(), panelX + (int)(24 * uiScale), drawY, iconFont, GOLD);
        std::string line = ability->name + "  (" + hotbarSlotText + ")";
        DrawText(line.c_str(), panelX + (int)(64 * uiScale), drawY, nameFont, WHITE);
        DrawText(ability->description.c_str(), panelX + (int)(64 * uiScale), drawY + (int)(nameFont * 1.15f), descFont, LIGHTGRAY);
        drawY += rowHeight;
    }
}

void DrawMagicTab(int panelX, int panelY, int panelW, int panelH)
{
    float uiScale = GetUIScale();
    int drawY = panelY + GetMenuTabBarHeight() + (int)(24 * uiScale);
    DrawText("No spells known yet.", panelX + (int)(24 * uiScale), drawY, (int)(26 * uiScale), LIGHTGRAY);
    DrawText("Spellcasting isn't wired into the game loop yet.", panelX + (int)(24 * uiScale), drawY + (int)(36 * uiScale), (int)(20 * uiScale), GRAY);
}

void DrawCharacterTab(int panelX, int panelY, int panelW, int panelH, const Player& player)
{
    float uiScale = GetUIScale();
    int drawY = panelY + GetMenuTabBarHeight() + (int)(24 * uiScale);
    int contentBottom = panelY + panelH - GetMenuFooterHeight();
    int padding = (int)(24 * uiScale);

    int titleFont = (int)(34 * uiScale);
    int lineFont = (int)(24 * uiScale);
    int smallFont = (int)(20 * uiScale);

    DrawText(player.name.c_str(), panelX + padding, drawY, titleFont, GOLD);
    drawY += (int)(titleFont * 1.3f);

    std::string classLine = player.className + "  |  Level " + std::to_string(player.level);
    DrawText(classLine.c_str(), panelX + padding, drawY, lineFont, WHITE);
    drawY += (int)(lineFont * 1.4f);

    const RaceData* race = FindRace(player.raceId);
    std::string raceName = race != nullptr ? race->name : "Unknown";
    std::string raceLine = "Race: " + raceName + "   Birthsign: " + player.birthsign;
    DrawText(raceLine.c_str(), panelX + padding, drawY, lineFont, VIOLET);
    drawY += (int)(lineFont * 1.6f);

    static const char* attrNames[] = { "STR", "END", "AGI", "INT", "WIL", "PER", "LCK" };
    static const Attribute attrs[] = { ATTRIBUTE_STR, ATTRIBUTE_END, ATTRIBUTE_AGI, ATTRIBUTE_INT, ATTRIBUTE_WIL, ATTRIBUTE_PER, ATTRIBUTE_LCK };
    std::string attrLine = "";
    for (int i = 0; i < 7; i++)
    {
        attrLine += std::string(attrNames[i]) + ":" + std::to_string(player.getAttributeValue(attrs[i])) + "   ";
    }
    DrawText(attrLine.c_str(), panelX + padding, drawY, lineFont, SKYBLUE);
    drawY += (int)(lineFont * 1.8f);

    DrawText("FEATS / BOONS:", panelX + padding, drawY, lineFont, GOLD);
    drawY += (int)(lineFont * 1.4f);
    if (player.activeFeats.empty() && player.activeBoons.empty())
    {
        DrawText("None yet.", panelX + padding + (int)(10 * uiScale), drawY, smallFont, LIGHTGRAY);
        drawY += (int)(smallFont * 1.4f);
    }
    for (size_t i = 0; i < player.activeFeats.size() && drawY <= contentBottom; i++)
    {
        const Feat* feat = FindFeat(player.activeFeats[i]);
        std::string name = feat != nullptr ? feat->name : "Unknown Feat";
        DrawText(name.c_str(), panelX + padding + (int)(10 * uiScale), drawY, smallFont, WHITE);
        drawY += (int)(smallFont * 1.4f);
    }
    for (size_t i = 0; i < player.activeBoons.size() && drawY <= contentBottom; i++)
    {
        const Feat* boon = FindFeat(player.activeBoons[i]);
        std::string name = boon != nullptr ? boon->name : "Unknown Boon";
        DrawText(name.c_str(), panelX + padding + (int)(10 * uiScale), drawY, smallFont, ORANGE);
        drawY += (int)(smallFont * 1.4f);
    }

    int skillColumnX = panelX + panelW / 2 + padding;
    int skillDrawY = panelY + GetMenuTabBarHeight() + (int)(24 * uiScale);
    DrawText("SKILLS:", skillColumnX, skillDrawY, lineFont, GOLD);
    skillDrawY += (int)(lineFont * 1.6f);
    for (size_t i = 0; i < player.skills.size() && i < G_SKILL_TYPES.size(); i++)
    {
        if (skillDrawY > contentBottom)
        {
            break;
        }
        std::string skillLine = G_SKILL_TYPES[i].name + ": " + std::to_string(player.skills[i].level);
        DrawText(skillLine.c_str(), skillColumnX, skillDrawY, smallFont, LIGHTGRAY);
        skillDrawY += (int)(smallFont * 1.3f);
    }
}