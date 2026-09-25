#include "EnemyTurns.h"
#include "EnemyFactory.h"

std::vector<Enemy*> AdvanceEnemyEnergy(std::vector<Enemy>& enemies)
{
    std::vector<Enemy*> ready;

    for (size_t i = 0; i < enemies.size(); i++)
    {
        Enemy& enemy = enemies[i];
        if (enemy.isDead)
        {
            continue;
        }

        enemy.energy += GetEnemySpeed(enemy);
        while (enemy.energy >= ACTION_THRESHOLD)
        {
            ready.push_back(&enemy);
            enemy.energy -= ACTION_THRESHOLD;
        }
    }

    return ready;
}