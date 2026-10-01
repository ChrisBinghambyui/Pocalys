#include "RosterData.h"

std::vector<Player> G_ROSTER;

void AddToRoster(const Player& player)
{
    Player returned = player;
    returned.hp = returned.maxHp;
    returned.stamina = returned.maxStamina;
    returned.mana = returned.maxMana;
    returned.attackCooldown = 0.0f;
    G_ROSTER.push_back(returned);
}

bool TakeFromRoster(int index, Player& out)
{
    if (index < 0 || index >= (int)G_ROSTER.size())
    {
        return false;
    }
    out = G_ROSTER[index];
    G_ROSTER.erase(G_ROSTER.begin() + index);
    return true;
}