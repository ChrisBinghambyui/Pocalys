#include "ItemData.h"
#include <vector>

// Kept minimal for now, expand as the loot table grows
std::vector<ItemArchetype> G_ITEM_ARCHETYPES = {
    {
        "iron_helmet",
        "Iron Helmet",
        "A dented iron cap, better than nothing.",
        200,
        SLOT_HELMET,
        1, 0, 0, 0
    },
    {
        "rusty_shortsword",
        "Rusty Shortsword",
        "Pitted and dull, but it still holds an edge.",
        201,
        SLOT_MAIN_HAND,
        0, 1, 6, 0
    },
    {
        "iron_buckler",
        "Iron Buckler",
        "A small round shield, dented but steady.",
        202,
        SLOT_OFF_HAND,
        1, 0, 0, 0,
        true
    }
};