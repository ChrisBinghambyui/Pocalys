#include "VariantData.h"

std::vector<CreatureVariant> G_CREATURE_VARIANTS = {
    // Ranged specialist. Trades some STR for AGI/PER and picks up bow access.
    { "archer",  "",        " Archer",        -5,  0, 10,  0,  0,  5,  0,  0,  2, 1.0f,  { 14, 15, 16 } },
    // Properly armed and armored instead of relying on natural weapons.
    { "warrior", "",        " Warrior",        8,  8, -3,  0,  0,  0,  0,  1,  0, 1.05f, { 3, 5, 6 } },
    // Small flat bump across the board, common low-tier upgrade.
    { "veteran", "",        " Veteran",        3,  3,  3,  2,  2,  2,  2,  0,  0, 1.0f,  {}, 0, 100, -10 },
    // Larger flat bump, no gear change. Prefix reads better than suffix ("Master Skeleton").
    { "master",  "Master ", "",                6,  6,  5,  6,  6,  4,  4,  1,  1, 1.1f,  {}, 1, 40, 25 },
    // Biggest generic elite tier: bigger, tougher, more dangerous, no gear change.
    { "lord",    "",        " Lord",          10, 10,  6,  6,  6,  4,  6,  2,  1, 1.3f,  {}, 2, 15, 25 },
    // Flavor/stat variant only. No disease system exists in code yet, this does NOT apply
    // any on-hit effect, it's just a tougher, slower, sicklier-statted version until that's built.
    { "plague_bearer", "",  " Plague-Bearer",  0,  6, -4,  0,  0,  0,  0,  1, -1, 1.1f,  {} }
};

const CreatureVariant* FindVariant(const std::string& id)
{
    for (size_t i = 0; i < G_CREATURE_VARIANTS.size(); i++)
    {
        if (G_CREATURE_VARIANTS[i].id == id)
        {
            return &G_CREATURE_VARIANTS[i];
        }
    }
    return nullptr;
}
