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
// Continuing the sequence for the new archetypes below. Placeholder indices, not verified
// against raygui's real GuiIconName enum, swap for the correct values when you check.
const int ICON_ZOMBIE = 189;
const int ICON_GHOST = 190;
const int ICON_VAMPIRE = 191;
const int ICON_LICH = 192;
const int ICON_KNIGHT = 193;
const int ICON_DRAGON = 194;
const int ICON_MUMMY = 195;
const int ICON_PEASANT = 196;
const int ICON_SOLDIER = 197;
const int ICON_BOWMAN = 198;
const int ICON_GRIFFIN = 199;
const int ICON_CAVALIER = 200;
const int ICON_CLERIC = 201;
const int ICON_ANGEL = 202;
const int ICON_SPRITE = 203;
const int ICON_GREMLIN = 204;
const int ICON_GOLEM = 205;
const int ICON_MAGE = 206;
const int ICON_NAGA = 207;
const int ICON_DJINN = 208;
const int ICON_TITAN = 209;
const int ICON_GOBLIN = 210;
const int ICON_WOLF_RIDER = 211;
const int ICON_ORC = 212;
const int ICON_OGRE = 213;
const int ICON_ROC = 214;
const int ICON_CYCLOPS = 215;
const int ICON_IMP = 216;
const int ICON_FAMILIAR = 217;
const int ICON_HORNED_DEMON = 218;
const int ICON_HELL_HOUND = 219;
const int ICON_CERBERUS = 220;
const int ICON_SUCCUBUS = 221;
const int ICON_DEVIL = 222;
const int ICON_FIEND = 223;
const int ICON_NIGHTMARE = 224;
const int ICON_UNICORN = 225;
const int ICON_MINOTAUR = 226;
const int ICON_RAIDER = 227;
const int ICON_HYDRA = 228;

std::vector<EnemyArchetype> G_ENEMY_ARCHETYPES = {
    // ==========================================
    // ORIGINAL 9 (rat/insect/skeleton/undead/beast/warlock/construct/horror/guardian)
    // ==========================================
    {
        "ratnik_scavenger", "Ratnik Scavenger",
        "A hunched, twitching rodent wielding a rusty file. Smells of rot and damp copper.",
        ICON_RAT,
        15, 15, 25, 10, 10, 10, 15,
        1, 8.0f,
        { 0 }, 0, 2, 1, 1,
        { "rat", "living" },
        'r', BROWN, 120, BRAIN_SKITTISH, 6
    },
    {
        "grave_mite", "Grave Mite Swarm",
        "A chitinous carpet of biting insects that feeds on subterranean decay.",
        ICON_BUG,
        10, 10, 20, 5, 10, 5, 10,
        0, 3.0f,
        {}, -1, -1, 0, 0,
        { "insect", "living" },
        'm', DARKBROWN, 130, BRAIN_MINDLESS, 4, false
    },
    {
        "moldering_skeleton", "Moldering Skeleton",
        "Brittle bones held together by old grudge-magic. Clack-clacks rhythmically in the dark.",
        ICON_SKULL,
        25, 20, 15, 5, 20, 5, 10,
        1, 45.0f,
        { 0, 1, 3, 5 }, 0, 4, 1, 1,
        { "skeleton", "undead" },
        's', RAYWHITE, 100, BRAIN_MINDLESS, 5
    },
    {
        "ironclad_beetle", "Ironclad Beetle",
        "A heavy subterranean beetle with a shell thick enough to turn broken blades.",
        ICON_SHIELD,
        30, 35, 10, 5, 15, 5, 10,
        4, 90.0f,
        {}, -1, -1, 0, 0,
        { "insect", "living" },
        'b', SKYBLUE, 80, BRAIN_TERRITORIAL, 5
    },
    {
        "ashwood_stalker", "Ashwood Stalker",
        "A gaunt, pale predator with elongated limbs built for leaping from cave ceilings.",
        ICON_BEAST,
        30, 20, 40, 10, 15, 10, 15,
        1, 60.0f,
        {}, -1, -1, 0, 0,
        { "beast", "living" },
        'a', LIGHTGRAY, 130, BRAIN_PREDATOR, 10
    },
    {
        "bog_witch", "Bog Witch",
        "A muttering recluse steeped in stagnant mire water, tossing corrosive hexes.",
        ICON_MAGIC,
        15, 20, 15, 35, 30, 10, 10,
        0, 55.0f,
        { 5 }, 2, 5, 1, 1,
        { "warlock", "living" },
        'w', PURPLE, 100, BRAIN_CASTER, 7
    },
    {
        "rust_golem", "Rust Golem",
        "An abandoned mining engine reanimated by iron-eating lichen and residual arcane heat.",
        ICON_ANVIL,
        50, 50, 10, 5, 10, 5, 5,
        5, 400.0f,
        {}, -1, -1, 0, 0,
        { "construct" },
        'G', ORANGE, 70, BRAIN_MINDLESS, 4, true, { "archer" }
    },
    {
        "void_gazer", "Void Gazer",
        "A floating mass of unblinking eyes that distorts lighting and drains resolve.",
        ICON_EYE,
        20, 30, 20, 45, 40, 20, 15,
        2, 20.0f,
        {}, -1, -1, 0, 0,
        { "horror" },
        'e', VIOLET, 100, BRAIN_CASTER, 9, true, { "archer" }
    },
    {
        "dread_warden", "Dread Warden",
        "An ancient armored sentry whose blade drags against the flagstones with shrieking sparks.",
        ICON_SWORD,
        55, 55, 25, 15, 35, 10, 15,
        6, 220.0f,
        { 3, 9 }, 8, 14, 1, 2,
        { "guardian", "living" },
        'W', RED, 100, BRAIN_TERRITORIAL, 8
    },

    // ==========================================
    // NECROPOLIS
    // ==========================================
    {
        "zombie_shambler", "Zombie",
        "A rotted corpse animated by lingering grudge-magic, shambling forward without pause or purpose.",
        ICON_ZOMBIE,
        20, 30, 8, 3, 8, 3, 5,
        2, 70.0f,
        {}, -1, -1, 0, 0,
        { "zombie", "undead" },
        'z', Color{ 90, 110, 60, 255 }, 90, BRAIN_MINDLESS, 4,
        true, { "archer" }
    },
    {
        "hollow_wight", "Wight",
        "A cold, half-seen shape that drains warmth from anything it touches. Cunning enough to flee when alone and dying.",
        ICON_GHOST,
        12, 18, 35, 20, 25, 15, 15,
        1, 25.0f,
        {}, -1, -1, 0, 0,
        { "ghost", "undead" },
        'y', Color{ 160, 160, 200, 255 }, 110, BRAIN_PREDATOR, 7,
        true, { "archer", "warrior" }
    },
    {
        "crimson_vampire", "Vampire",
        "A gaunt noble drained pale by its own hunger, moving with an unnatural, unhurried grace.",
        ICON_VAMPIRE,
        30, 28, 35, 25, 25, 30, 20,
        2, 65.0f,
        { 0, 1 }, 4, 9, 0, 1,
        { "vampire", "undead" },
        'V', Color{ 140, 10, 20, 255 }, 110, BRAIN_PREDATOR, 9,
        true, {}
    },
    {
        "ashen_lich", "Lich",
        "A robed skeleton wreathed in cold necrotic light, hanging back to let lesser dead absorb the first blow.",
        ICON_LICH,
        10, 20, 15, 50, 45, 20, 15,
        1, 55.0f,
        {}, -1, -1, 0, 0,
        { "lich", "undead" },
        'L', Color{ 80, 200, 180, 255 }, 100, BRAIN_CASTER, 8,
        true, { "archer", "warrior" }
    },
    {
        "black_knight", "Black Knight",
        "A dead champion still holding its post, disciplined and unhurried even in undeath.",
        ICON_KNIGHT,
        45, 50, 25, 15, 35, 20, 15,
        5, 180.0f,
        { 3, 9 }, 6, 11, 1, 1,
        { "knight", "undead" },
        'K', Color{ 20, 20, 30, 255 }, 100, BRAIN_TERRITORIAL, 7,
        true, {}
    },
    {
        "bone_dragon", "Bone Dragon",
        "A dragon's husk, wings of tattered hide stretched over bone, driven by hunger alone.",
        ICON_DRAGON,
        60, 55, 30, 20, 30, 15, 15,
        7, 300.0f,
        {}, -1, -1, 0, 0,
        { "dragon", "undead" },
        'D', Color{ 230, 230, 220, 255 }, 100, BRAIN_MINDLESS, 10,
        true, { "archer", "warrior" }
    },
    {
        "restless_ghost", "Ghost",
        "A translucent shape that passes through the eye more than it's seen, cold air marking where it's been.",
        ICON_GHOST,
        5, 15, 30, 25, 30, 10, 15,
        0, 5.0f,
        {}, -1, -1, 0, 0,
        { "ghost", "undead" },
        'g', Color{ 200, 220, 230, 180 }, 120, BRAIN_PREDATOR, 8,
        true, { "archer", "warrior" }
    },
    {
        "wretched_mummy", "Mummy",
        "A bandage-wrapped corpse, slow and unhurried, trailing a stink that turns the stomach before the blow lands.",
        ICON_MUMMY,
        22, 35, 5, 5, 12, 3, 5,
        3, 75.0f,
        {}, -1, -1, 0, 0,
        { "mummy", "undead" },
        'u', Color{ 190, 175, 130, 255 }, 80, BRAIN_MINDLESS, 4,
        true, { "archer" }
    },

    // ==========================================
    // HAVEN
    // ==========================================
    {
        "haven_peasant", "Peasant",
        "A conscripted farmhand clutching a borrowed pitchfork, more afraid of the dark than anything in it.",
        ICON_PEASANT,
        10, 15, 15, 10, 10, 10, 10,
        0, 70.0f,
        { 7 }, 0, 2, 0, 1,
        { "peasant", "living" },
        'p', BROWN, 100, BRAIN_SKITTISH, 5,
        true, { "lord" }
    },
    {
        "haven_squire", "Squire",
        "A young soldier drilled on castle grounds, eager to prove himself against something that bleeds.",
        ICON_SOLDIER,
        25, 25, 20, 10, 15, 15, 10,
        1, 75.0f,
        { 1, 3 }, 2, 6, 1, 1,
        { "soldier", "living" },
        'q', Color{ 150, 150, 200, 255 }, 100, BRAIN_PREDATOR, 6,
        true, {}
    },
    {
        "haven_bowman", "Bowman",
        "A trained archer holding formation, more comfortable with distance between himself and the fight.",
        ICON_BOWMAN,
        15, 20, 35, 10, 15, 20, 10,
        1, 70.0f,
        { 14, 15 }, 2, 6, 1, 1,
        { "bowman", "living" },
        'o', DARKGREEN, 110, BRAIN_CASTER, 8,
        true, {}
    },
    {
        "haven_griffin", "Griffin",
        "A lion-eagle hybrid fiercely loyal to its nest, diving with a shriek that scatters lesser creatures.",
        ICON_GRIFFIN,
        30, 30, 45, 10, 20, 25, 15,
        2, 90.0f,
        {}, -1, -1, 0, 0,
        { "griffin", "living" },
        'F', Color{ 210, 180, 120, 255 }, 120, BRAIN_TERRITORIAL, 9,
        true, { "archer", "warrior" }
    },
    {
        "haven_cavalier", "Cavalier",
        "An armored rider bearing down at a full charge, lance leveled, warhorse snorting steam in the cold.",
        ICON_CAVALIER,
        45, 45, 30, 15, 25, 20, 15,
        4, 220.0f,
        { 7, 3 }, 4, 9, 1, 1,
        { "knight", "living" },
        'C', Color{ 60, 60, 160, 255 }, 110, BRAIN_PREDATOR, 8,
        true, {}
    },
    {
        "haven_cleric", "Cleric",
        "A temple-trained healer carrying a blunt mace more out of obligation than skill, murmuring prayers between blows.",
        ICON_CLERIC,
        15, 25, 15, 35, 40, 30, 15,
        1, 70.0f,
        { 5 }, 2, 6, 0, 1,
        { "cleric", "living" },
        'c', Color{ 230, 230, 255, 255 }, 100, BRAIN_CASTER, 7,
        true, {}
    },
    {
        "haven_angel", "Angel",
        "A towering, radiant warrior descended to carry out a judgment no mortal asked for. Undead unmake themselves trying to look away.",
        ICON_ANGEL,
        50, 55, 35, 30, 45, 35, 20,
        6, 150.0f,
        { 3, 9 }, 8, 14, 1, 1,
        { "angel", "living" },
        'A', GOLD, 110, BRAIN_PREDATOR, 10,
        true, {}
    },

    // ==========================================
    // ACADEMY
    // ==========================================
    {
        "academy_sprite", "Sprite",
        "A dust-winged fey no larger than a fist, darting through the dark faster than the eye can settle on it.",
        ICON_SPRITE,
        5, 10, 45, 25, 20, 20, 20,
        0, 3.0f,
        {}, -1, -1, 0, 0,
        { "sprite", "living" },
        'x', Color{ 255, 220, 120, 255 }, 140, BRAIN_SKITTISH, 9,
        true, { "archer", "warrior", "lord", "plague_bearer" }
    },
    {
        "academy_gremlin", "Gremlin",
        "A wiry tinkerer clutching a rusted wrench, swarming in numbers to make up for what it lacks in size.",
        ICON_GREMLIN,
        15, 15, 25, 20, 10, 10, 15,
        0, 40.0f,
        { 0, 2 }, 0, 3, 1, 1,
        { "gremlin", "living" },
        'k', Color{ 100, 180, 100, 255 }, 110, BRAIN_PREDATOR, 6,
        true, {}
    },
    {
        "academy_stone_golem", "Stone Golem",
        "A construct of animated rock and old binding-magic, plodding forward with no concern for what stands in its way.",
        ICON_GOLEM,
        35, 45, 5, 5, 15, 5, 5,
        4, 260.0f,
        {}, -1, -1, 0, 0,
        { "golem", "construct" },
        'O', Color{ 140, 140, 140, 255 }, 80, BRAIN_MINDLESS, 4,
        true, { "archer", "warrior", "plague_bearer" }
    },
    {
        "academy_mage", "Mage",
        "A robed scholar wielding a warded staff, more dangerous for what it hasn't cast yet than for what it has.",
        ICON_MAGE,
        15, 20, 15, 45, 35, 20, 15,
        1, 70.0f,
        { 5 }, 2, 7, 1, 1,
        { "mage", "living" },
        'M', Color{ 90, 90, 220, 255 }, 100, BRAIN_CASTER, 8,
        true, {}
    },
    {
        "academy_naga", "Naga",
        "A serpent-bodied guardian coiled around whatever it's sworn to protect, unhurried until that oath is threatened.",
        ICON_NAGA,
        35, 40, 25, 20, 25, 20, 15,
        3, 140.0f,
        { 7, 12 }, 4, 9, 1, 1,
        { "naga", "living" },
        'n', Color{ 60, 150, 90, 255 }, 100, BRAIN_TERRITORIAL, 7,
        true, {}
    },
    {
        "academy_djinn", "Djinn",
        "A smoke-bodied spirit bound into service, curved blade in hand, hovering just above the reach of a mortal swing.",
        ICON_DJINN,
        30, 30, 40, 30, 30, 20, 20,
        2, 60.0f,
        { 4, 8 }, 4, 9, 1, 1,
        { "djinn", "living" },
        'j', Color{ 100, 200, 220, 255 }, 120, BRAIN_PREDATOR, 9,
        true, {}
    },
    {
        "academy_titan", "Titan",
        "A giant crackling with stormlight, each footstep a small tremor, patient in the way only something this large can afford to be.",
        ICON_TITAN,
        55, 60, 25, 35, 35, 20, 15,
        6, 400.0f,
        { 9, 10 }, 6, 12, 1, 1,
        { "titan", "living" },
        'T', Color{ 240, 230, 100, 255 }, 100, BRAIN_PREDATOR, 10,
        true, {}
    },

    // ==========================================
    // STRONGHOLD
    // ==========================================
    {
        "stronghold_goblin", "Goblin",
        "A scrawny raider hooting for backup at the first sign of trouble, brave only in a crowd.",
        ICON_GOBLIN,
        15, 15, 20, 5, 5, 10, 10,
        0, 45.0f,
        { 0, 2 }, 0, 2, 1, 1,
        { "goblin", "living" },
        'g', Color{ 100, 130, 60, 255 }, 100, BRAIN_PREDATOR, 6,
        true, {}
    },
    {
        "stronghold_wolf_rider", "Wolf Rider",
        "A goblin lashed to the back of a snarling wolf, both of them faster and meaner than either alone.",
        ICON_WOLF_RIDER,
        20, 20, 40, 10, 10, 10, 10,
        1, 85.0f,
        { 2, 7 }, 0, 3, 0, 1,
        { "goblin", "beast", "living" },
        'w', Color{ 130, 130, 100, 255 }, 130, BRAIN_PREDATOR, 8,
        true, {}
    },
    {
        "stronghold_orc", "Orc",
        "A hulking brute swinging a spiked club with more enthusiasm than technique, and enough of it to matter.",
        ICON_ORC,
        35, 35, 15, 10, 15, 10, 10,
        1, 100.0f,
        { 5, 6 }, 2, 6, 1, 1,
        { "orc", "living" },
        'O', Color{ 90, 120, 70, 255 }, 100, BRAIN_PREDATOR, 6,
        true, { "archer" }
    },
    {
        "stronghold_ogre", "Ogre",
        "A slow, mountainous brawler that shrugs off wounds a smaller creature wouldn't survive, and hits back twice as hard.",
        ICON_OGRE,
        50, 55, 10, 5, 15, 5, 10,
        2, 220.0f,
        { 10, 13 }, 4, 9, 1, 1,
        { "ogre", "living" },
        'G', Color{ 150, 110, 60, 255 }, 90, BRAIN_PREDATOR, 6,
        true, {}
    },
    {
        "stronghold_roc", "Roc",
        "A storm-scarred bird of prey the size of a wagon, circling before it drops with a scream and a pair of grasping talons.",
        ICON_ROC,
        35, 35, 45, 10, 20, 20, 15,
        2, 100.0f,
        {}, -1, -1, 0, 0,
        { "roc", "beast", "living" },
        'R', Color{ 160, 90, 60, 255 }, 130, BRAIN_PREDATOR, 10,
        true, { "master", "lord", "veteran" }
    },
    {
        "stronghold_cyclops", "Cyclops",
        "A one-eyed giant that hurls boulders before you're close, then finishes the job barehanded once you are.",
        ICON_CYCLOPS,
        60, 60, 15, 10, 20, 10, 10,
        4, 380.0f,
        {}, -1, -1, 0, 0,
        { "cyclops", "living" },
        'C', Color{ 170, 140, 90, 255 }, 90, BRAIN_TERRITORIAL, 8,
        true, { "archer" }
    },

    // ==========================================
    // INFERNO
    // ==========================================
    {
        "inferno_imp", "Imp",
        "A cackling little horror that darts in to claw and vanishes before the counterswing lands.",
        ICON_IMP,
        10, 10, 30, 15, 10, 10, 20,
        0, 20.0f,
        {}, -1, -1, 0, 0,
        { "demon" },
        'i', Color{ 200, 40, 40, 255 }, 130, BRAIN_PREDATOR, 6,
        true, { "master", "lord", "veteran" }
    },
    {
        "inferno_familiar", "Familiar",
        "A bound imp-spirit tethered to a summoner somewhere unseen, twitchy and quick to flee alone.",
        ICON_FAMILIAR,
        8, 8, 35, 20, 15, 10, 20,
        0, 8.0f,
        {}, -1, -1, 0, 0,
        { "demon", "sprite" },
        'f', Color{ 220, 80, 80, 255 }, 140, BRAIN_SKITTISH, 7,
        true, { "master", "lord" }
    },
    {
        "inferno_horned_demon", "Horned Demon",
        "A brutish, low-ranking soldier of the Pit, quick to anger and slow to think past the nearest throat.",
        ICON_HORNED_DEMON,
        35, 35, 15, 10, 15, 10, 10,
        1, 110.0f,
        { 6, 11 }, 4, 9, 1, 1,
        { "demon" },
        'H', Color{ 160, 30, 30, 255 }, 100, BRAIN_PREDATOR, 6,
        true, { "archer" }
    },
    {
        "inferno_hell_hound", "Hell Hound",
        "A charred, panting beast wreathed in low flame, running down anything that turns its back.",
        ICON_HELL_HOUND,
        25, 20, 45, 10, 10, 10, 10,
        1, 55.0f,
        {}, -1, -1, 0, 0,
        { "demon", "beast" },
        'h', Color{ 210, 60, 20, 255 }, 130, BRAIN_PREDATOR, 8,
        true, { "master", "lord", "veteran" }
    },
    {
        "inferno_cerberus", "Cerberus",
        "A three-headed hound the size of a warhorse, each head snapping independently at whatever's closest.",
        ICON_CERBERUS,
        40, 35, 40, 15, 15, 10, 10,
        2, 140.0f,
        {}, -1, -1, 0, 0,
        { "demon", "beast" },
        'C', Color{ 180, 40, 20, 255 }, 130, BRAIN_PREDATOR, 9,
        true, { "master", "lord", "veteran" }
    },
    {
        "inferno_succubus", "Succubus",
        "A honey-voiced horror that lingers just out of reach, whispering promises no one should believe.",
        ICON_SUCCUBUS,
        20, 25, 30, 25, 30, 35, 15,
        1, 65.0f,
        { 4, 6 }, 4, 9, 1, 1,
        { "succubus", "demon" },
        's', Color{ 190, 50, 100, 255 }, 110, BRAIN_CASTER, 9,
        true, {}
    },
    {
        "inferno_devil", "Devil",
        "A towering, forked-tailed horror wielding a trident wreathed in black flame, patient in the way only something ancient in its cruelty can be.",
        ICON_DEVIL,
        55, 50, 35, 25, 30, 20, 15,
        5, 240.0f,
        { 7, 12 }, 6, 12, 1, 1,
        { "devil", "demon" },
        'D', Color{ 140, 20, 20, 255 }, 110, BRAIN_PREDATOR, 10,
        true, {}
    },
    {
        "inferno_fiend", "Fiend",
        "A stout, muscle-bound horror one rank beneath the Devils, still strong enough that rank hardly matters.",
        ICON_FIEND,
        45, 45, 30, 20, 25, 15, 15,
        4, 190.0f,
        { 5, 10 }, 6, 11, 1, 1,
        { "fiend", "demon" },
        'F', Color{ 160, 40, 30, 255 }, 110, BRAIN_PREDATOR, 9,
        true, {}
    },
    {
        "inferno_nightmare", "Nightmare",
        "A hell-bred horse wreathed in black smoke, hooves striking sparks off stone that shouldn't spark at all.",
        ICON_NIGHTMARE,
        30, 30, 50, 10, 15, 10, 15,
        2, 150.0f,
        {}, -1, -1, 0, 0,
        { "nightmare", "demon" },
        'n', Color{ 60, 20, 60, 255 }, 130, BRAIN_PREDATOR, 8,
        true, { "master", "lord", "veteran" }
    },

    // ==========================================
    // STRAGGLERS: RAMPART / DUNGEON
    // ==========================================
    {
        "rampart_unicorn", "Unicorn",
        "A horned white steed radiating quiet menace toward anything that means the grove harm.",
        ICON_UNICORN,
        30, 35, 45, 15, 30, 30, 20,
        2, 150.0f,
        {}, -1, -1, 0, 0,
        { "unicorn", "beast", "living" },
        'u', Color{ 240, 240, 250, 255 }, 110, BRAIN_TERRITORIAL, 9,
        true, { "archer", "warrior", "plague_bearer" }
    },
    {
        "dungeon_minotaur", "Minotaur",
        "A bull-headed guardian pacing the same coiled stretch of tunnel it's paced for longer than it remembers why.",
        ICON_MINOTAUR,
        45, 40, 20, 10, 15, 10, 10,
        3, 200.0f,
        { 5, 6 }, 5, 9, 1, 1,
        { "minotaur", "beast", "living" },
        'M', Color{ 120, 80, 60, 255 }, 100, BRAIN_TERRITORIAL, 6,
        true, { "archer" }
    },
    {
        "dungeon_raider", "Raider",
        "A wiry skirmisher darting between shadows, striking once and vanishing before the counterblow lands.",
        ICON_RAIDER,
        25, 25, 40, 15, 15, 20, 15,
        1, 90.0f,
        { 1, 4 }, 2, 7, 1, 1,
        { "raider", "beast", "living" },
        'd', Color{ 90, 60, 100, 255 }, 120, BRAIN_PREDATOR, 8,
        true, {}
    },
    {
        "dungeon_hydra", "Hydra",
        "A many-headed serpent coiled around its hoard, each head watching a different corner of the dark.",
        ICON_HYDRA,
        40, 55, 15, 5, 10, 5, 5,
        3, 300.0f,
        {}, -1, -1, 0, 0,
        { "hydra", "beast", "living" },
        'H', Color{ 60, 100, 60, 255 }, 90, BRAIN_TERRITORIAL, 7,
        true, { "master", "lord", "veteran" }
    }
};


void ResolveEnemyArchetypeDefaults()
{
    static const std::vector<std::string> backlineIds = {
        "bog_witch", "academy_mage", "void_gazer", "ashen_lich",
        "haven_bowman", "haven_cleric", "inferno_succubus"
    };
    static const std::vector<std::string> skirmisherIds = {
        "ratnik_scavenger", "stronghold_wolf_rider", "dungeon_raider",
        "inferno_imp", "academy_djinn", "academy_gremlin"
    };
    static const std::vector<std::string> squadBrainIds = {
        "stronghold_goblin", "stronghold_orc", "haven_squire", "haven_bowman",
        "bog_witch", "academy_mage", "stronghold_wolf_rider", "academy_gremlin",
        "inferno_horned_demon", "haven_cavalier"
    };

    for (auto& archetype : G_ENEMY_ARCHETYPES)
    {
        for (const auto& id : backlineIds)
        {
            if (archetype.id == id)
            {
                archetype.role = ROLE_BACKLINE;
            }
        }
        for (const auto& id : skirmisherIds)
        {
            if (archetype.id == id)
            {
                archetype.role = ROLE_SKIRMISHER;
            }
        }
        for (const auto& id : squadBrainIds)
        {
            if (archetype.id == id)
            {
                archetype.brainId = "squad_martial";
            }
        }
    }
}

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
    { "dread_warden", 10, 99, 5, 5 },

    // Necropolis
    { "zombie_shambler", 2, 7, 35, 3 },
    { "hollow_wight", 4, 9, 18, 4 },
    { "crimson_vampire", 6, 11, 12, 4 },
    { "ashen_lich", 7, 12, 10, 4 },
    { "black_knight", 9, 99, 8, 4 },
    { "bone_dragon", 12, 99, 4, 3 },
    { "restless_ghost", 3, 8, 20, 3 },
    { "wretched_mummy", 4, 10, 22, 3 },

    // Haven
    { "haven_peasant", 1, 4, 45, -5 },
    { "haven_squire", 1, 6, 35, 0 },
    { "haven_bowman", 2, 7, 25, 2 },
    { "haven_griffin", 3, 8, 15, 3 },
    { "haven_cleric", 4, 9, 12, 2 },
    { "haven_cavalier", 6, 99, 10, 4 },
    { "haven_angel", 12, 99, 3, 2 },

    // Academy
    { "academy_sprite", 1, 5, 40, -3 },
    { "academy_gremlin", 1, 6, 35, 0 },
    { "academy_stone_golem", 3, 8, 18, 2 },
    { "academy_mage", 4, 9, 14, 3 },
    { "academy_naga", 6, 11, 10, 3 },
    { "academy_djinn", 8, 99, 8, 3 },
    { "academy_titan", 13, 99, 3, 2 },

    // Stronghold
    { "stronghold_goblin", 1, 4, 45, -5 },
    { "stronghold_wolf_rider", 2, 6, 30, 0 },
    { "stronghold_orc", 2, 7, 30, 2 },
    { "stronghold_ogre", 5, 10, 18, 3 },
    { "stronghold_roc", 6, 11, 14, 3 },
    { "stronghold_cyclops", 9, 99, 8, 4 },

    // Inferno
    { "inferno_imp", 1, 5, 40, -4 },
    { "inferno_familiar", 1, 5, 30, -3 },
    { "inferno_horned_demon", 2, 7, 30, 2 },
    { "inferno_hell_hound", 3, 8, 22, 3 },
    { "inferno_cerberus", 5, 10, 14, 3 },
    { "inferno_succubus", 6, 11, 10, 3 },
    { "inferno_devil", 10, 99, 6, 4 },
    { "inferno_fiend", 8, 99, 9, 4 },
    { "inferno_nightmare", 6, 11, 12, 3 },

    // Stragglers
    { "rampart_unicorn", 4, 9, 14, 3 },
    { "dungeon_minotaur", 5, 10, 14, 3 },
    { "dungeon_raider", 2, 7, 22, 2 },
    { "dungeon_hydra", 7, 12, 10, 3 }
};
