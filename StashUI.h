#pragma once
#include <vector>

enum StashUIAction
{
    STASH_UI_NONE,
    STASH_UI_BEGIN, // Enter: loadout chosen, start the run
    STASH_UI_BACK   // Esc: back to patron selection
};

struct StashLoadoutState
{
    int selected = 0;
    std::vector<bool> packed; // Parallel to G_STASH.items. True = comes along on the next run.
};

// One frame of input and drawing for the "pack your bag" screen. Owns its own BeginDrawing/EndDrawing,
// same as UpdateAndDrawTitleScreen.
StashUIAction UpdateAndDrawStashLoadout(StashLoadoutState& state);