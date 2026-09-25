#pragma once
#include <string>
#include <vector>

struct ArrowType
{
    int id;
    std::string name;
    std::string description;
    int damageBonus; // Small and deliberately capped, arrow quality should nudge damage, not define it
};

extern std::vector<ArrowType> G_ARROW_TYPES;

// Null if no arrow type has this id.
const ArrowType* FindArrowType(int id);