#pragma once
#include <vector>
#include <string>
#include "Item.h"
#include "ItemData.h"
#include "SkillData.h"

struct SkillState {
    int level;
    int xp;
};

struct Player {
    std::string name;
    std::string className;
    std::string birthsign;

    int x;
    int y;

    // Core Character Progression
    int level = 1;
    int levelProgress = 0;

    // Feat & Boon Banking
    int unspentFeatPoints = 1; // Start with 1 Feat at character creation
    int unspentBoonPoints = 0;

    int hp;
    int maxHp;
    int stamina;
    int maxStamina;
    int mana;
    int maxMana;

    // Core Attributes
    int str;
    int end;
    int agi;
    int intel;
    int wil;
    int per;
    int lck;

    std::vector<Item> inventory;
    Item equippedSlots[SLOT_SINGLE_COUNT];
    std::vector<Item> equippedAmulets;
    std::vector<Item> equippedRings;

    std::vector<SkillState> skills;

    // Active character unlocks
    std::vector<int> activeFeats;
    std::vector<int> activeBoons;

    int getAttributeValue(Attribute attr) const {
        switch (attr) {
        case ATTRIBUTE_STR: return str;
        case ATTRIBUTE_END: return end;
        case ATTRIBUTE_AGI: return agi;
        case ATTRIBUTE_INT: return intel;
        case ATTRIBUTE_WIL: return wil;
        case ATTRIBUTE_PER: return per;
        case ATTRIBUTE_LCK: return lck;
        case ATTRIBUTE_NONE:
        default: return 0;
        }
    }

    void initSkills(int baseLevel = 5) {
        skills.assign(G_SKILL_TYPES.size(), { baseLevel, 0 });
    }

    int getXpForNextSkillLevel(int currentLevel) const {
        return currentLevel * 10;
    }

    bool addSkillXp(int skillId, int amount, bool isMajorOrMinor = false) {
        if (skillId < 0 || skillId >= skills.size()) return false;

        bool leveledUp = false;
        skills[skillId].xp += amount;

        int xpNeeded = getXpForNextSkillLevel(skills[skillId].level);

        while (skills[skillId].xp >= xpNeeded) {
            skills[skillId].xp -= xpNeeded;
            skills[skillId].level++;
            leveledUp = true;

            if (isMajorOrMinor) {
                levelProgress++;
            }

            xpNeeded = getXpForNextSkillLevel(skills[skillId].level);
        }

        return leveledUp;
    }

    bool readyToLevelUp() const {
        return levelProgress >= 10;
    }

    // Handles increasing character level, granting Feat/Boon points
    void executeLevelUp() {
        if (!readyToLevelUp()) return;

        levelProgress -= 10;
        level++;

        // Grant 1 Feat per level up by default
        unspentFeatPoints++;

        // Grant 1 Boon point every 5 levels
        if (level % 5 == 0) {
            unspentBoonPoints++;
        }
    }
};