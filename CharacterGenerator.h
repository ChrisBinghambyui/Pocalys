#pragma once
#include <string>
#include <vector>
#include "Item.h"
#include "ItemData.h"

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
};

struct ClassData {
    std::string name;
    std::string description;
    int strMod, endMod, agiMod, intelMod, wilMod, perMod, lckMod;
    int startingWeaponTypeId;
    int startingMaterialTier;
    std::string startingArmorArchetype;
};

struct BirthsignData {
    std::string name;
    std::string description;
    int strMod, endMod, agiMod, intelMod, wilMod, perMod, lckMod;
};

CharacterProfile GenerateRandomCharacter();
std::vector<CharacterProfile> GenerateTavernCandidates(int count = 6);