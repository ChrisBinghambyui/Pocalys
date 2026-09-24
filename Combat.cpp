#include "Combat.h"
#include "EnemyFactory.h"
#include "raylib.h"

int CalculateUnarmedDamage(const Player& attacker)
{
    int damage = attacker.str / 10;
    damage += GetRandomValue(-1, 1);
    if (damage < 1)
    {
        damage = 1;
    }
    return damage;
}

bool ResolveBumpAttack(Player& attacker, Enemy& defender, std::string& actionMessage)
{
    int damage = CalculateUnarmedDamage(attacker);
    defender.hp -= damage;

    if (defender.hp <= 0)
    {
        defender.hp = 0;
        defender.isDead = true;
        actionMessage = "The " + GetEnemyDisplayName(defender) + " collapses!";
        return true;
    }
    else
    {
        actionMessage = "You hit the " + GetEnemyDisplayName(defender) + " for " + std::to_string(damage) + " damage!";
        return false;
    }
}