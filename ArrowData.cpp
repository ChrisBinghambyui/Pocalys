#include "ArrowData.h"

std::vector<ArrowType> G_ARROW_TYPES = {
    { 0, "Practice Arrows",  "Blunted and blemished, meant for the training yard.", -1 },
    { 1, "Rough Arrows",     "Hand-fletched, a little crooked.",                     0 },
    { 2, "Iron Arrows",      "A steady, reliable point.",                            1 },
    { 3, "Steel Arrows",     "Tempered and true.",                                   2 },
    { 4, "Silvertip Arrows", "Silver-headed, favored against the unliving.",         2 },
    { 5, "Broadhead Arrows", "Wide-bladed, meant to open a wound.",                  3 }
};

const ArrowType* FindArrowType(int id)
{
    for (size_t i = 0; i < G_ARROW_TYPES.size(); i++)
    {
        if (G_ARROW_TYPES[i].id == id)
        {
            return &G_ARROW_TYPES[i];
        }
    }
    return nullptr;
}