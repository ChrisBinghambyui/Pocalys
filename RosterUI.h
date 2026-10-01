#pragma once

enum RosterUIAction
{
    ROSTER_UI_NONE,
    ROSTER_UI_BEGIN, // Enter or click: lead the next expedition with this veteran
    ROSTER_UI_BACK   // Esc: back to the tavern
};

struct RosterViewState
{
    int selected = 0;
};

// One frame of input and drawing for the veterans screen. Owns its own BeginDrawing/EndDrawing.
// When the result is ROSTER_UI_BEGIN, outPickedIndex is the index into G_ROSTER.
RosterUIAction UpdateAndDrawRoster(RosterViewState& state, int& outPickedIndex);