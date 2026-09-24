#include "CharacterGenerator.h"
#include "WeaponData.h"
#include "MaterialData.h"
#include <raylib.h>
#include <algorithm>

static const std::vector<std::string> G_FIRST_NAMES = {
    "Alden", "Brak", "Caelen", "Darian", "Eldrin", "Fintan", "Garrick",
    "Halia", "Isolda", "Joran", "Kael", "Lyra", "Maren", "Niamh",
    "Orin", "Pyra", "Quinn", "Rowan", "Soren", "Thalia", "Vael", "Wren"
};

static const std::vector<std::string> G_EPITHETS = {
    "Ironheart", "Shadowwalker", "Stormweaver", "the Bold", "Frostriver",
    "Spellbound", "Bloodaxe", "Nightshade", "Silvertongue", "the Silent",
    "Swiftblade", "Oakshield", "Sunsworn", "Duskbringer"
};

static const std::vector<ClassData> G_CLASSES = {
        { "Knight",     "An armored defender trained in heavy arms.",       65, 55, 35, 25, 25, 35, 30, 3, 5, "iron_helmet",
        { SKILL_BLUNT, SKILL_AXE, SKILL_LONG_BLADE, SKILL_HEAVY_ARMOR, SKILL_ATHLETICS },
        { SKILL_BLOCK, SKILL_MEDIUM_ARMOR, SKILL_SPEECHCRAFT, SKILL_ACROBATICS, SKILL_RESTORATION } },
    { "Mage",       "A master of arcane forces and sorcery.",           20, 25, 30, 75, 65, 30, 30, 0, 6, "",
        { SKILL_DESTRUCTION, SKILL_ALTERATION, SKILL_MYSTICISM, SKILL_CONJURATION, SKILL_ALCHEMY },
        { SKILL_ILLUSION, SKILL_RESTORATION, SKILL_ENCHANT, SKILL_UNARMORED, SKILL_SHORT_BLADE } },
    { "Rogue",      "An agile skirmisher relying on speed and luck.",   30, 35, 60, 40, 25, 55, 40, 1, 4, "",
        { SKILL_SHORT_BLADE, SKILL_SNEAK, SKILL_SECURITY, SKILL_ACROBATICS, SKILL_SPEECHCRAFT },
        { SKILL_LIGHT_ARMOR, SKILL_MARKSMAN, SKILL_MERCANTILE, SKILL_ATHLETICS, SKILL_ILLUSION } },
    { "Barbarian",  "A ferocious warrior wielding heavy axes.",         65, 60, 40, 20, 25, 25, 35, 11, 5, "",
        { SKILL_AXE, SKILL_BLUNT, SKILL_ATHLETICS, SKILL_UNARMORED, SKILL_HAND_TO_HAND },
        { SKILL_MEDIUM_ARMOR, SKILL_BLOCK, SKILL_ACROBATICS, SKILL_SPEAR, SKILL_MARKSMAN } },
    { "Spellsword", "A versatile combatant blending blade and spell.",  45, 45, 35, 55, 45, 25, 30, 4, 6, "",
        { SKILL_DESTRUCTION, SKILL_LONG_BLADE, SKILL_MEDIUM_ARMOR, SKILL_ALTERATION, SKILL_BLOCK },
        { SKILL_RESTORATION, SKILL_CONJURATION, SKILL_ATHLETICS, SKILL_HEAVY_ARMOR, SKILL_MYSTICISM } },
    { "Ranger",     "A sharp-eyed hunter and marksman.",                35, 45, 65, 30, 25, 25, 35, 14, 0, "",
        { SKILL_MARKSMAN, SKILL_SNEAK, SKILL_LIGHT_ARMOR, SKILL_ATHLETICS, SKILL_SHORT_BLADE },
        { SKILL_ACROBATICS, SKILL_LONG_BLADE, SKILL_ALCHEMY, SKILL_BLOCK, SKILL_RESTORATION } },
    { "Paladin",    "A holy champion wielding hammers with fortitude.", 60, 55, 30, 25, 50, 35, 30, 10, 5, "iron_helmet",
        { SKILL_BLUNT, SKILL_HEAVY_ARMOR, SKILL_RESTORATION, SKILL_BLOCK, SKILL_ATHLETICS },
        { SKILL_LONG_BLADE, SKILL_MEDIUM_ARMOR, SKILL_SPEECHCRAFT, SKILL_MYSTICISM, SKILL_ARMORER } },
    { "Crusader",   "A disciplined spearman built for dungeon delves.", 55, 55, 35, 30, 55, 35, 25, 7, 3, "",
        { SKILL_SPEAR, SKILL_HEAVY_ARMOR, SKILL_RESTORATION, SKILL_BLOCK, SKILL_ATHLETICS },
        { SKILL_BLUNT, SKILL_MEDIUM_ARMOR, SKILL_SPEECHCRAFT, SKILL_MYSTICISM, SKILL_CONJURATION } }
};

static const std::vector<BirthsignData> G_BIRTHSIGNS = {
    { "The Warrior",  "+10 Strength, +5 Endurance.",       10,  5,  0,  0,  0,  0,  0 },
    { "The Mage",     "+10 Intelligence, +5 Willpower.",    0,  0,  0, 10,  5,  0,  0 },
    { "The Thief",    "+10 Agility, +5 Luck.",            0,  0, 10,  0,  0,  0,  5 },
    { "The Lady",     "+10 Endurance, +5 Willpower.",      0, 10,  0,  0,  5,  0,  0 },
    { "The Steed",    "+10 Agility, +5 Strength.",         5,  0, 10,  0,  0,  0,  0 },
    { "The Tower",    "+10 Endurance, +5 Perception.",     0, 10,  0,  0,  0,  5,  0 },
    { "The Ritual",   "+10 Willpower, +5 Intelligence.",   0,  0,  0,  5, 10,  0,  0 },
    { "The Shadow",   "+10 Perception, +5 Agility.",       0,  0,  5,  0,  0, 10,  0 }
};

static const int CLASS_ATTRIBUTE_VARIANCE = 5;
static const int CREATION_ATTRIBUTE_MIN = 10;
static const int CREATION_ATTRIBUTE_MAX = 75;

// Class spread +/- variance, then race and birthsign mods, clamped to the creation range.
static int RollAttribute(int classBase, int raceMod, int signMod)
{
    int variance = GetRandomValue(-CLASS_ATTRIBUTE_VARIANCE, CLASS_ATTRIBUTE_VARIANCE);
    int value = classBase + variance + raceMod + signMod;
    if (value < CREATION_ATTRIBUTE_MIN)
    {
        value = CREATION_ATTRIBUTE_MIN;
    }
    if (value > CREATION_ATTRIBUTE_MAX)
    {
        value = CREATION_ATTRIBUTE_MAX;
    }
    return value;
}

CharacterProfile GenerateRandomCharacter() {
    CharacterProfile profile;

    std::string fname = G_FIRST_NAMES[GetRandomValue(0, (int)G_FIRST_NAMES.size() - 1)];
    std::string lname = G_EPITHETS[GetRandomValue(0, (int)G_EPITHETS.size() - 1)];
    profile.name = fname + " " + lname;

    const ClassData& cls = G_CLASSES[GetRandomValue(0, (int)G_CLASSES.size() - 1)];
    profile.className = cls.name;
    profile.majorSkills = cls.majorSkills;
    profile.minorSkills = cls.minorSkills;

    const BirthsignData& sign = G_BIRTHSIGNS[GetRandomValue(0, (int)G_BIRTHSIGNS.size() - 1)];
    profile.birthsign = sign.name;

    const RaceData& race = G_RACES[GetRandomValue(0, (int)G_RACES.size() - 1)];
    profile.raceId = race.id;
    profile.raceSkillBonuses = RollRaceSkillBonuses(race);

    profile.str = RollAttribute(cls.baseStr, race.strMod, sign.strMod);
    profile.end = RollAttribute(cls.baseEnd, race.endMod, sign.endMod);
    profile.agi = RollAttribute(cls.baseAgi, race.agiMod, sign.agiMod);
    profile.intel = RollAttribute(cls.baseIntel, race.intelMod, sign.intelMod);
    profile.wil = RollAttribute(cls.baseWil, race.wilMod, sign.wilMod);
    profile.per = RollAttribute(cls.basePer, race.perMod, sign.perMod);
    profile.lck = RollAttribute(cls.baseLck, race.lckMod, sign.lckMod);

    profile.maxHp = profile.end / 5;
    profile.hp = profile.maxHp;

    profile.maxMana = profile.intel / 5;
    profile.mana = profile.maxMana;

    profile.maxStamina = (profile.end + profile.str + profile.agi + 40) / 10;
    profile.stamina = profile.maxStamina;


    for (int i = 0; i < SLOT_SINGLE_COUNT; i++) {
        profile.equippedSlots[i] = Item();
    }

    if (cls.startingWeaponTypeId >= 0) {
        Item weaponItem;
        weaponItem.weaponTypeId = cls.startingWeaponTypeId;
        weaponItem.materialTier = cls.startingMaterialTier;
        weaponItem.condition = CONDITION_PRISTINE;
        profile.equippedSlots[SLOT_MAIN_HAND] = weaponItem;
    }

    if (!cls.startingArmorArchetype.empty()) {
        Item armorItem;
        armorItem.archetypeId = cls.startingArmorArchetype;
        armorItem.condition = CONDITION_PRISTINE;
        for (const auto& arch : G_ITEM_ARCHETYPES) {
            if (arch.id == cls.startingArmorArchetype) {
                if (arch.slot < SLOT_SINGLE_COUNT) {
                    profile.equippedSlots[arch.slot] = armorItem;
                }
                break;
            }
        }
    }

    Item backup;
    backup.archetypeId = "rusty_shortsword";
    profile.inventory.push_back(backup);

    return profile;
}

std::vector<CharacterProfile> GenerateTavernCandidates(int count) {
    std::vector<CharacterProfile> candidates;
    for (int i = 0; i < count; i++) {
        candidates.push_back(GenerateRandomCharacter());
    }
    return candidates;
}