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
    { "Knight",     "An armored defender trained in heavy arms.",       10, 10,  0, -5,  0,  5,  0, 3, 5, "iron_helmet" },
    { "Mage",       "A master of arcane forces and sorcery.",         -5, -5,  5, 15, 10,  0,  0, 0, 6, "" },
    { "Rogue",      "An agile skirmisher relying on speed and luck.",    0,  0, 15,  0,  0, 10,  5, 1, 4, "" },
    { "Barbarian",  "A ferocious warrior wielding heavy axes.",         15, 10,  5,-10, -5,  0,  5, 11, 5, "" },
    { "Spellsword", "A versatile combatant blending blade and spell.",   5,  0,  5, 10,  5,  0,  0, 4, 6, "" },
    { "Ranger",     "A sharp-eyed hunter and marksman.",                0,  5, 10,  0,  0, 15,  0, 14, 0, "" },
    { "Paladin",    "A holy champion wielding hammers with fortitude.", 10,  5, -5,  0, 10,  5,  0, 10, 5, "iron_helmet" },
    { "Crusader",   "A disciplined spearman built for dungeon delves.", 10,  5,  0,  5,  5,  0,  0, 7, 3, "" }
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


CharacterProfile GenerateRandomCharacter() {
    CharacterProfile profile;

    std::string fname = G_FIRST_NAMES[GetRandomValue(0, (int)G_FIRST_NAMES.size() - 1)];
    std::string lname = G_EPITHETS[GetRandomValue(0, (int)G_EPITHETS.size() - 1)];
    profile.name = fname + " " + lname;

    const ClassData& cls = G_CLASSES[GetRandomValue(0, (int)G_CLASSES.size() - 1)];
    profile.className = cls.name;

    const BirthsignData& sign = G_BIRTHSIGNS[GetRandomValue(0, (int)G_BIRTHSIGNS.size() - 1)];
    profile.birthsign = sign.name;

    int baseStr = GetRandomValue(25, 40);
    int baseEnd = GetRandomValue(25, 40);
    int baseAgi = GetRandomValue(25, 40);
    int baseInt = GetRandomValue(25, 40);
    int baseWil = GetRandomValue(25, 40);
    int basePer = GetRandomValue(25, 40);
    int baseLck = GetRandomValue(25, 40);

    profile.str = std::max(10, baseStr + cls.strMod + sign.strMod);
    profile.end = std::max(10, baseEnd + cls.endMod + sign.endMod);
    profile.agi = std::max(10, baseAgi + cls.agiMod + sign.agiMod);
    profile.intel = std::max(10, baseInt + cls.intelMod + sign.intelMod);
    profile.wil = std::max(10, baseWil + cls.wilMod + sign.wilMod);
    profile.per = std::max(10, basePer + cls.perMod + sign.perMod);
    profile.lck = std::max(10, baseLck + cls.lckMod + sign.lckMod);

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