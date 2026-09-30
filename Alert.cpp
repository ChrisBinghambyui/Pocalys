#include "Alert.h"

static int GetAlertEventAmount(AlertEventType eventType)
{
    if (eventType == ALERT_EVENT_COMBAT_HIT)
    {
        return 2;
    }
    if (eventType == ALERT_EVENT_ENEMY_KILLED)
    {
        return 1;
    }
    if (eventType == ALERT_EVENT_SPOTTED)
    {
        return 1;
    }
    return 0;
}

void RaiseAlert(int& floorAlert, AlertEventType eventType)
{
    floorAlert += GetAlertEventAmount(eventType);
    if (floorAlert > ALERT_MAX)
    {
        floorAlert = ALERT_MAX;
    }
}

bool IsFloorWary(int floorAlert)
{
    return floorAlert >= ALERT_WARY_THRESHOLD;
}

bool IsFloorSearching(int floorAlert)
{
    return floorAlert >= ALERT_SEARCHING_THRESHOLD;
}