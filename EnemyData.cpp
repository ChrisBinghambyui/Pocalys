#include "EnemyData.h"

// Raylib icon enum values mapped directly for UI rendering
const int ICON_RAT = 180;
const int ICON_BUG = 181;
const int ICON_SKULL = 182;
const int ICON_SHIELD = 183;
const int ICON_BEAST = 184;
const int ICON_MAGIC = 185;
const int ICON_ANVIL = 186;
const int ICON_EYE = 187;
const int ICON_SWORD = 188;

std::vector<EnemyArchetype> G_ENEMY_ARCHETYPES = {
    {
        "ratnik_scavenger", "Ratnik Scavenger",
        "A hunched, twitching rodent wielding a rusty file. Smells of rot and damp copper.",
        ICON_RAT,
        15, 15, 25, 10, 10, 10, 15,
        1, 8.0f,
        { 0 }, 0, 2, 1, 1,
        { "vermin", "living" },
        'r', BROWN, 120
    },
    {
        "grave_mite", "Grave Mite Swarm",
        "A chitinous carpet of biting insects that feeds on subterranean decay.",
        ICON_BUG,
        10, 10, 20, 5, 10, 5, 10,
        0, 3.0f,
        {}, -1, -1, 0, 0,
        { "vermin", "living" },
        'm', DARKBROWN, 130
    },
    {
        "moldering_skeleton", "Moldering Skeleton",
        "Brittle bones held together by old grudge-magic. Clack-clacks rhythmically in the dark.",
        ICON_SKULL,
        25, 20, 15, 5, 20, 5, 10,
        1, 45.0f,
        { 0, 1, 3, 5 }, 0, 4, 1, 1,
        { "skeleton", "undead" },
        's', RAYWHITE, 100
    },
    {
        "ironclad_beetle", "Ironclad Beetle",
        "A heavy subterranean beetle with a shell thick enough to turn broken blades.",
        ICON_SHIELD,
        30, 35, 10, 5, 15, 5, 10,
        4, 90.0f,
        {}, -1, -1, 0, 0,
        { "chitinguard", "living" },
        'b', SKYBLUE, 80
    },
    {
        "ashwood_stalker", "Ashwood Stalker",
        "A gaunt, pale predator with elongated limbs built for leaping from cave ceilings.",
        ICON_BEAST,
        30, 20, 40, 10, 15, 10, 15,
        1, 60.0f,
        {}, -1, -1, 0, 0,
        { "nightprowler", "living" },
        'a', LIGHTGRAY, 130
    },
    {
        "bog_witch", "Bog Witch",
        "A muttering recluse steeped in stagnant mire water, tossing corrosive hexes.",
        ICON_MAGIC,
        15, 20, 15, 35, 30, 10, 10,
        0, 55.0f,
        { 5 }, 2, 5, 1, 1,
        { "hexbound", "living" },
        'w', PURPLE, 100
    },
    {
        "rust_golem", "Rust Golem",
        "An abandoned mining engine reanimated by iron-eating lichen and residual arcane heat.",
        ICON_ANVIL,
        50, 50, 10, 5, 10, 5, 5,
        5, 400.0f,
        {}, -1, -1, 0, 0,
        { "mechanism" },
        'G', ORANGE, 70
    },
    {
        "void_gazer", "Void Gazer",
        "A floating mass of unblinking eyes that distorts lighting and drains resolve.",
        ICON_EYE,
        20, 30, 20, 45, 40, 20, 15,
        2, 20.0f,
        {}, -1, -1, 0, 0,
        { "aberration" },
        'e', VIOLET, 100
    },
    {
        "dread_warden", "Dread Warden",
        "An ancient armored sentry whose blade drags against the flagstones with shrieking sparks.",
        ICON_SWORD,
        55, 55, 25, 15, 35, 10, 15,
        6, 220.0f,
        { 3, 9 }, 8, 14, 1, 2,
        { "warden", "living" },
        'W', RED, 100
    }
};

std::vector<SpawnRule> G_SPAWN_RULES = {
    // Floor 1-3: Shallow danger
    { "ratnik_scavenger", 1, 4, 50, -5 },
    { "grave_mite", 1, 3, 40, -10 },
    { "moldering_skeleton", 2, 6, 30, 5 },

    // Floor 4-7: Mid depth
    { "ironclad_beetle", 3, 8, 20, 5 },
    { "ashwood_stalker", 4, 9, 15, 5 },
    { "bog_witch", 5, 10, 10, 5 },

    // Floor 8+: Deep horror
    { "rust_golem", 7, 99, 10, 3 },
    { "void_gazer", 8, 99, 8, 4 },
    { "dread_warden", 10, 99, 5, 5 }
};