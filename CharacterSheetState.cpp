#include "CharacterSheetState.h"
#include <iostream>
#include <iomanip>
#include <sstream>

std::string CharacterSheetState::buildProgressBar(int current, int max, int width) const {
    if (max <= 0) max = 1;
    float percentage = static_cast<float>(current) / static_cast<float>(max);
    int filledWidth = static_cast<int>(percentage * width);
    if (filledWidth < 0) filledWidth = 0;
    if (filledWidth > width) filledWidth = width;

    std::string bar = "[";
    for (int i = 0; i < width; ++i) {
        if (i < filledWidth) bar += "=";
        else bar += " ";
    }
    bar += "] " + std::to_string(current) + "/" + std::to_string(max);
    return bar;
}

void CharacterSheetState::render(const Player& player) const {
    if (!m_isOpen) return;

    std::cout << "========================================================================================\n";
    std::cout << "                                  CHARACTER SHEET                                       \n";
    std::cout << "========================================================================================\n";

    // --- GENERAL INFORMATION ---
    std::cout << " Name: " << std::left << std::setw(20) << player.name
        << " Level: " << std::setw(5) << player.level
        << " Class: " << player.className << "\n";
    std::cout << " Birthsign: " << player.birthsign << "\n";
    std::cout << "----------------------------------------------------------------------------------------\n";

    // --- VITALS & RESOURCES ---
    std::cout << " HP: " << std::left << std::setw(28) << buildProgressBar(player.hp, player.maxHp)
        << " SP: " << std::left << std::setw(28) << buildProgressBar(player.stamina, player.maxStamina)
        << " MP: " << buildProgressBar(player.mana, player.maxMana) << "\n";
    std::cout << "----------------------------------------------------------------------------------------\n";

    // --- ATTRIBUTES & ACTIVE FEATS / BOONS ---
    std::cout << " ATTRIBUTES                                ACTIVE FEATS / BOONS\n";
    std::cout << " ----------                                --------------------\n";

    static const char* attrNames[] = {
        "Strength", "Endurance", "Agility", "Intelligence", "Willpower", "Perception", "Luck"
    };

    // Attribute enum mapping strictly matching Player.h
    static const Attribute attributes[] = {
        ATTRIBUTE_STR, ATTRIBUTE_END, ATTRIBUTE_AGI,
        ATTRIBUTE_INT, ATTRIBUTE_WIL, ATTRIBUTE_PER, ATTRIBUTE_LCK
    };

    auto findFeat = [](int featId) -> const Feat* {
        for (const auto& feat : G_FEATS) {
            if (feat.id == featId) return &feat;
        }
        return nullptr;
        };

    size_t featIndex = 0;
    for (int i = 0; i < 7; ++i) {
        int attrVal = player.getAttributeValue(attributes[i]);
        std::cout << " " << std::left << std::setw(14) << attrNames[i]
            << ": " << std::right << std::setw(3) << attrVal << "   |   ";

        if (featIndex < player.activeFeats.size()) {
            int featId = player.activeFeats[featIndex];
            const Feat* f = findFeat(featId);
            if (f) {
                std::cout << "- " << f->name;
            }
            else {
                std::cout << "- Feat #" << featId;
            }
            featIndex++;
        }
        else if (i == 0 && player.activeFeats.empty()) {
            std::cout << "None";
        }
        std::cout << "\n";
    }

    // Print remaining active feats if player has more than 7
    while (featIndex < player.activeFeats.size()) {
        int featId = player.activeFeats[featIndex];
        const Feat* f = findFeat(featId);
        if (f) {
            std::cout << "                         |   - " << f->name << "\n";
        }
        else {
            std::cout << "                         |   - Feat #" << featId << "\n";
        }
        featIndex++;
    }

    std::cout << "----------------------------------------------------------------------------------------\n";

    // --- SKILLS LIST ---
    std::cout << " SKILLS & PROFICIENCIES\n";
    std::cout << " ----------------------\n";

    int countOnLine = 0;
    for (size_t i = 0; i < player.skills.size(); ++i) {
        const auto& skill = player.skills[i];

        std::cout << "   " << std::left << std::setw(18) << ("Skill " + std::to_string(i + 1))
            << " Lvl: " << std::right << std::setw(3) << skill.level;
        countOnLine++;

        if (countOnLine % 2 == 0) {
            std::cout << "\n";
        }
        else {
            std::cout << "  |";
        }
    }
    if (countOnLine % 2 != 0) std::cout << "\n";

    std::cout << "========================================================================================\n";
    std::cout << " [TAB / ESC] Close Character Sheet\n";
    std::cout << "========================================================================================\n";
}

bool CharacterSheetState::handleInput(char key) {
    if (key == 9 || key == 27 || key == 't' || key == 'T') {
        toggle();
        return true;
    }
    return m_isOpen;
}