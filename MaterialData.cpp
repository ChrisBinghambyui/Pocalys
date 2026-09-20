#include "MaterialData.h"
#include <vector>

std::vector<MaterialTier> G_MATERIAL_TIERS = {
    { 0,  0, AFFINITY_LIGHT, "Green Wood",        "Sharpened branches, barely better than nothing.",                       -2, 0, 0.7f,  20 },
    { 1,  0, AFFINITY_HEAVY, "Cured Bone",         "Dried and honed, the bone is brittle but good in a pinch.",             -2, 0, 1.2f,  20 },
    { 2,  1, AFFINITY_LIGHT, "Knapped Flint",      "Chipped to an edge, snaps under real pressure.",                    -1, 0, 0.8f,  16 },
    { 3,  1, AFFINITY_HEAVY, "Cold-Worked Copper", "Hammered soft metal, heavier but won't shatter.",                   -1, 0, 1.2f,  16 },
    { 4,  2, AFFINITY_LIGHT, "Bronze",             "Tin-alloyed, the first metal that trusts a fast swing.",            -1, 0, 0.85f, 12 },
    { 5,  2, AFFINITY_HEAVY, "Iron",               "The soldier's baseline, dense and dependable.",                     -1, 0, 1.15f, 12 },
    { 6,  3, AFFINITY_LIGHT, "Spring Steel",       "Flexible temper, favors a quick draw.",                              0, 0, 0.85f, 9  },
    { 7,  3, AFFINITY_HEAVY, "Blacksteel",         "Folded and quenched heavy, built for a shield wall.",                0, 1, 1.2f,  9  },
    { 8,  4, AFFINITY_LIGHT, "Silver",             "Cold and thin, bites harder into anything unliving.",                1, 0, 0.85f, 7  },
    { 9,  4, AFFINITY_HEAVY, "Cold Iron",          "Meteoric ore the old fey roads avoid, sits heavy in the hand.",      1, 0, 1.2f,  7  },
    { 10, 5, AFFINITY_LIGHT, "Skysteel",           "Fallen-star ore, unnaturally light for its strength.",               1, 1, 0.75f, 5  },
    { 11, 5, AFFINITY_HEAVY, "Grave Iron",         "The result of iron infused with blood.",             1, 1, 1.25f, 5  },
    { 12, 6, AFFINITY_LIGHT, "Mithril",            "Sings when struck, light enough to forget you're wearing it.",       2, 1, 0.6f,  3  },
    { 13, 6, AFFINITY_HEAVY, "Adamant",            "Dense past reason, most hammers break before the metal bends.",        2, 2, 1.3f,  3  },
    { 14, 7, AFFINITY_LIGHT, "Duskglass",          "Cast where the planes run thin, cuts light instead of catching it.", 2, 1, 0.7f,  2  },
    { 15, 7, AFFINITY_HEAVY, "Godsteel",           "Smelted near a dormant god's seat, few temples allow it twice.",     2, 2, 1.3f,  2  },
    { 16, 8, AFFINITY_LIGHT, "Hardlight",        "Pure magic, forced into temporal form.",      3, 2, 0.5f,  1  },
    { 17, 8, AFFINITY_HEAVY, "Heartiron",          "Tempered with a measure of the smith's own soul, costly to make.",   3, 3, 1.3f,  1  },
    { 18, 9, AFFINITY_LIGHT, "Voidmetal",          "Untouched by rust or time, unsettling how little it weighs.",        3, 2, 0.5f,  0  },
    { 19, 9, AFFINITY_HEAVY, "Chronesteel",        "Never dulls, never ages, found only in inexplicable ruins",      3, 3, 1.3f,  0  },
    { 20, -1, AFFINITY_NONE, "Sundered",           "Legendary. No known forge produces it. Whatever made it wasn't mortal.", 4, 4, 1.0f, 0  }
};