#pragma once

// Alert never decreases within a floor: once a floor gets loud, it stays that way for the rest of
// the visit. Resetting it on re-entry after a rest is a rulebook mechanic for later, once resting
// exists in the game loop.
const int ALERT_MAX = 12;
const int ALERT_WARY_THRESHOLD = 3;      // Idle/Wander stop being options, Patrol takes over instead
const int ALERT_SEARCHING_THRESHOLD = 6; // Detection radius gets a flat bonus on top of Patrol

enum AlertEventType
{
    ALERT_EVENT_COMBAT_HIT,   // An enemy landed or took a hit
    ALERT_EVENT_ENEMY_KILLED, // A body left behind, more likely to be found
    ALERT_EVENT_SPOTTED       // An enemy just acquired a hostile target for the first time
};

// Raises floorAlert for this event, clamped to ALERT_MAX. Never lowers it.
void RaiseAlert(int& floorAlert, AlertEventType eventType);

bool IsFloorWary(int floorAlert);
bool IsFloorSearching(int floorAlert);