#pragma once

#include "Player.h"
#include "FeatData.h"
#include <string>

class CharacterSheetState {
public:
    CharacterSheetState() = default;

    void toggle() { m_isOpen = !m_isOpen; }
    bool isOpen() const { return m_isOpen; }

    void render(const Player& player) const;
    bool handleInput(char key);

private:
    bool m_isOpen = false;

    std::string buildProgressBar(int current, int max, int width = 15) const;
};