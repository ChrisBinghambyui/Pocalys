#pragma once
#include <vector>
#include "Player.h"

// Characters who extracted alive and are waiting at the tavern. A character leaves the roster the moment
// a run starts with them and only returns by extracting, so quitting or dying mid-run means they are gone.
extern std::vector<Player> G_ROSTER;

// Adds a character who extracted alive. Vitals are restored and transient combat state cleared.
// Their inventory should already be banked in the stash, they keep what they have equipped.
void AddToRoster(const Player& player);

// Moves the character at this index out of the roster into `out`. False if the index is bad.
bool TakeFromRoster(int index, Player& out);