#pragma once
#include "Player.h"
#include "Enemy.h"
#include <string>

int CalculateUnarmedDamage(const Player& attacker);
bool ResolveBumpAttack(Player& attacker, Enemy& defender, std::string& actionMessage);