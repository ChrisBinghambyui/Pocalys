#pragma once
#include <string>
#include <vector>
#include "Item.h"
#include "ItemData.h"
#include "SkillData.h"
#include "RaceData.h"

struct CharacterProfile {
    std::string name;
    std::string className;
    std::string birthsign;

    int str;
    int end;
    int agi;
    int intel;
    int wil;
    int per;
    int lck;

    int maxHp;
    int hp;
    int maxStamina;
    int stamina;
    int maxMana;
    int mana;

    std::vector<Item> inventory;
    Item equippedSlots[SLOT_SINGLE_COUNT];

    std::vector<int> majorSkills; // Skill ids that start at 40
    std::vector<int> minorSkills; // Skill ids that start at 25

    std::string raceId;                       // Key into G_RACES
    std::vector<SkillBonus> raceSkillBonuses; // Resolved at roll time so the tavern can show them
};

struct ClassData {
    std::string name;
    std::string description;
    int baseStr, baseEnd, baseAgi, baseIntel, baseWil, basePer, baseLck; // Flat class spread, before variance, race, and birthsign
    int startingWeaponTypeId;
    int startingMaterialTier;
    std::string startingArmorArchetype;
    std::vector<int> majorSkills; // 5 skill ids
    std::vector<int> minorSkills; // 5 skill ids
};

struct BirthsignData {
    std::string name;
    std::string description;
    int strMod, endMod, agiMod, intelMod, wilMod, perMod, lckMod;
};

CharacterProfile GenerateRandomCharacter();
std::vector<CharacterProfile> GenerateTavernCandidates(int count = 6);