#pragma once
#include "Player.h"

enum MenuTab
{
    MENU_TAB_INVENTORY,
    MENU_TAB_ABILITIES,
    MENU_TAB_MAGIC,
    MENU_TAB_CHARACTER,
    MENU_TAB_COUNT
};

int GetMenuTabBarHeight();
int GetMenuFooterHeight();

// Index of the tab under the point, or -1 if the point is outside the tab bar. Same layout as DrawMenuTabBar.
int GetMenuTabAtPoint(int panelX, int panelY, int panelW, int pointX, int pointY);

void DrawMenuTabBar(int panelX, int panelY, int panelW, MenuTab currentTab);
void DrawAbilitiesTab(int panelX, int panelY, int panelW, int panelH, const Player& player);
void DrawMagicTab(int panelX, int panelY, int panelW, int panelH);
void DrawCharacterTab(int panelX, int panelY, int panelW, int panelH, const Player& player);