#pragma once

// Saves what outlives a run. Right now that is the stash. The roster and graveyard join it later as
// more record tags in the same file. It does not save the run itself (floors, enemies, position).

// Writes savegame.dat in the working directory. False if the file could not be written.
bool SaveMetaGame();

// Reads savegame.dat into the game's persistent state. False if there is no file, it is unreadable, or
// its version is not one this build understands. In every failure case the state is left untouched.
bool LoadMetaGame();