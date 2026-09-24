#pragma once

enum TitleAction
{
    TITLE_ACTION_NONE,
    TITLE_ACTION_NEW_GAME,
    TITLE_ACTION_OPTIONS,
    TITLE_ACTION_QUIT
};

void InitTitleScreen();                 // Call once after InitWindow
void ResetTitleScreen();                // New random theme, clears effect state. Call when returning to the title.
TitleAction UpdateAndDrawTitleScreen(); // One frame of title input and drawing
