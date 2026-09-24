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
        "ratnik_scavenger",
        "Ratnik Scavenger",
        "A hunched, twitching rodent wielding a rusty file. Smells of rot and damp copper.",
        ICON_RAT,
        12, 3, 3, 1, 0,
        { 0 }, 0, 2, 1, 1, 'r', BROWN // Dagger only, wood/bone/flint tier scavenged junk
    },
    {
        "grave_mite",
        "Grave Mite Swarm",
        "A chitinous carpet of biting insects that feeds on subterranean decay.",
        ICON_BUG,
        8, 2, 2, 1, 1,
        {}, -1, -1, 0, 0, 'm', DARKBROWN // Bites only, no gear
    },
    {
        "moldering_skeleton",
        "Moldering Skeleton",
        "Brittle bones held together by old grudge-magic. Clack-clacks rhythmically in the dark.",
        ICON_SKULL,
        18, 4, 5, 1, 1,
        { 0, 1, 3, 5 }, 0, 4, 1, 1, 's', RAYWHITE // Dagger/Shortsword/Longsword/Mace, ancient grave-goods quality
    },
    {
        "ironclad_beetle",
        "Ironclad Beetle",
        "A heavy subterranean beetle with a shell thick enough to turn broken blades.",
        ICON_SHIELD,
        30, 6, 4, 1, 4,
        {}, -1, -1, 0, 0, 'b', SKYBLUE // Natural carapace and mandibles only
    },
    {
        "ashwood_stalker",
        "Ashwood Stalker",
        "A gaunt, pale predator with elongated limbs built for leaping from cave ceilings.",
        ICON_BEAST,
        25, 5, 8, 2, 1,
        {}, -1, -1, 0, 0, 'a', LIGHTGRAY // Claws only
    },
    {
        "bog_witch",
        "Bog Witch",
        "A muttering recluse steeped in stagnant mire water, tossing corrosive hexes.",
        ICON_MAGIC,
        22, 4, 7, 2, 0,
        { 5 }, 2, 5, 1, 1, 'w', PURPLE // Gnarled mace-staff, flint through iron tier
    },
    {
        "rust_golem",
        "Rust Golem",
        "An abandoned mining engine reanimated by iron-eating lichen and residual arcane heat.",
        ICON_ANVIL,
        45, 8, 10, 2, 5,
        {}, -1, -1, 0, 0, 'G', ORANGE // Fists and machinery, no carried weapon
    },
    {
        "void_gazer",
        "Void Gazer",
        "A floating mass of unblinking eyes that distorts lighting and drains resolve.",
        ICON_EYE,
        35, 7, 12, 3, 2,
        {}, -1, -1, 0, 0, 'e', VIOLET // No hands to hold anything
    },
    {
        "dread_warden",
        "Dread Warden",
        "An ancient armored sentry whose blade drags against the flagstones with a shrieking sparks.",
        ICON_SWORD,
        60, 10, 14, 3, 6,
        { 3, 9 }, 8, 14, 1, 2, 'W', RED // Longsword/Greatsword, Silver through Duskglass quality, sometimes dual
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