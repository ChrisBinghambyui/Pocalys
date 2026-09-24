#include "RaceData.h"
#include "SkillData.h"
#include "raylib.h"

static const std::vector<int> G_MAGIC_SCHOOLS = {
    SKILL_DESTRUCTION, SKILL_ALTERATION, SKILL_ILLUSION,
    SKILL_CONJURATION, SKILL_MYSTICISM, SKILL_RESTORATION
};

static const std::vector<int> G_NONCOMBAT_SKILLS = {
    SKILL_ARMORER, SKILL_ENCHANT, SKILL_ALCHEMY, SKILL_SECURITY,
    SKILL_SNEAK, SKILL_ACROBATICS, SKILL_MERCANTILE, SKILL_SPEECHCRAFT
};

// Attribute order: STR, END, AGI, INT, WIL, PER, LCK. SPD is not modeled in the roguelike.
// Last field is the racial feat id (see the RACIAL FEATS block in FeatData.cpp).
std::vector<RaceData> G_RACES = {
    // Elven
    { "solarirum", "Solarirum", "Gilt-skinned scholar-casters from the oldest elven city-states. Brilliant, brittle, and sure of it.",
        -5, -5, 0, 10, 5, 0, 0,
        { { G_MAGIC_SCHOOLS, 10, 1 } },
        200 },
    { "sylvarirum", "Sylvarirum", "Wiry forest elves who spend their youth wandering alone. Nothing in the trees hears them coming.",
        -10, -5, 10, 0, 0, 0, 0,
        { { { SKILL_MARKSMAN }, 10, 0 }, { { SKILL_SNEAK }, 5, 0 } },
        201 },
    { "vethanirum", "Vethanirum", "Borderborn elves who take no one's traditions too seriously, least of all their own.",
        -5, -5, -5, 5, 5, 5, 0,
        { { G_MAGIC_SCHOOLS, 5, 0 } },
        202 },
    { "ashirum", "Ashirum", "Ember-eyed elves raised on shaking ground. Comfort is temporary, adapting is permanent.",
        0, -5, 5, 5, 0, -5, -5,
        { { G_MAGIC_SCHOOLS, 10, 1 }, { { SKILL_LONG_BLADE, SKILL_SHORT_BLADE }, 5, 1 } },
        203 },

        // Dwarven
        { "apisdrenn", "Apisdrenn", "Mountain dwarves, stubborn as their stone. Refusing a gift from them is nearly impossible.",
            5, 10, -5, 0, 0, -5, 0,
            { { { SKILL_HEAVY_ARMOR }, 10, 0 }, { { SKILL_BLUNT, SKILL_AXE }, 10, 1 } },
            204 },
        { "apiskeld", "Apiskeld", "Forge Lords of the deep holds. A working anvil sounds like music to them.",
            5, 5, -5, 5, 0, -5, 0,
            { { { SKILL_ENCHANT }, 10, 0 }, { { SKILL_BLUNT, SKILL_HEAVY_ARMOR }, 5, 0 } },
            205 },
        { "apisveldir", "Apisveldir", "Gem Seekers who think fine stones are listening. Never argue with one about a sapphire.",
            -5, -5, -5, 5, 0, 5, 5,
            { { { SKILL_MERCANTILE, SKILL_ENCHANT }, 10, 0 }, { { SKILL_SECURITY }, 5, 0 } },
            206 },
        { "kreln", "Kreln", "Deepstone dwarves from the crushing dark below the sea. Plated, glowing, and squinting at the sun.",
            5, 10, 0, 0, 0, -5, -5,
            { { { SKILL_ATHLETICS }, 10, 0 }, { { SKILL_HEAVY_ARMOR, SKILL_BLUNT }, 5, 1 } },
            207 },

            // Human
            { "imperial", "Imperial", "Lowland administrators, merchants, and soldiers. They won the continent with paperwork and coin.",
                0, 5, -5, -5, -5, 5, 5,
                { { { SKILL_SPEECHCRAFT }, 10, 0 }, { { SKILL_MERCANTILE }, 5, 0 } },
                208 },
            { "nordal", "Nordal", "Tall, scarred northerners who value strength, plain speech, and finishing what they start.",
                10, 5, 0, -5, -5, -5, 0,
                { { { SKILL_BLUNT, SKILL_LONG_BLADE }, 10, 1 }, { G_MAGIC_SCHOOLS, 5, 0, SKILL_BONUS_SET } },
                209 },
            { "sunblade", "Sunblade", "Wanderers who treat the sword as high art. Home waits until they carry something worth bringing back.",
                10, 0, 5, -5, -5, 0, -5,
                { { { SKILL_LONG_BLADE }, 10, 0 }, { { SKILL_BLOCK }, 5, 0 } },
                210 },
            { "ashveld", "Ashveld", "Highlanders who live where the ground sometimes kills. Practical, improvising, unimpressed by magic.",
                0, 5, 5, -5, -5, -5, 5,
                { { { SKILL_ATHLETICS }, 10, 0 }, { { SKILL_ALCHEMY, SKILL_ARMORER, SKILL_SECURITY }, 5, 1 } },
                211 },

                // Orcish and others
                { "stoneguard", "Stoneguard", "Iron-blooded orcs of the war-band and the forge. Authority is earned, never inherited.",
                    10, 5, 0, 0, 0, -5, -5,
                    { { { SKILL_BLUNT, SKILL_AXE }, 10, 1 }, { { SKILL_HEAVY_ARMOR }, 5, 0 } },
                    212 },
                { "gorirum", "Gorirum", "Longer-eared orcs who claim the elves as distant kin. The elves are still working on their reply.",
                    5, 0, -5, 5, 5, -5, -5,
                    { { G_MAGIC_SCHOOLS, 5, 2 }, { { SKILL_LONG_BLADE }, 10, 0 } },
                    213 },
                { "veildrift", "Veildrift", "Pale, half-present people found at the edges of old catastrophes. They stopped asking where from.",
                    -5, -5, 0, 5, 10, -5, 0,
                    { { { SKILL_MYSTICISM }, 10, 0 }, { { SKILL_SNEAK }, 5, 0 } },
                    214 },
                { "murrak", "Murrak", "Grey-skinned, violet-eyed, and stocky. Magic runs in their blood whether or not they studied it.",
                    0, 5, 0, -10, 5, -5, 0,
                    { { { SKILL_DESTRUCTION, SKILL_ILLUSION }, 10, 1 }, { { SKILL_ENCHANT }, 5, 0 } },
                    215 },

                    // Beastborn
                    { "bovari", "Bovari", "Four-armed, horned ox-folk. Good-natured builders and cooks whose horns tell their life story.",
                        10, 5, -5, -5, 0, 0, 0,
                        { { { SKILL_ATHLETICS }, 10, 0 }, { G_NONCOMBAT_SKILLS, 5, 2 } },
                        216 },
                    { "naukin", "Naukin", "Ratfolk who slipped out of the wizard academies. They remember what the spells were used for.",
                        -5, -5, 10, 0, 0, -5, 0,
                        { { { SKILL_SNEAK }, 10, 0 }, { { SKILL_SECURITY }, 5, 0 } },
                        217 },
                    { "arantza", "Arantza", "Six-armed spider-folk keeping stories older than most kingdoms. Eight eyes, and they use all of them.",
                        5, 0, 5, -5, -5, -5, 0,
                        { { { SKILL_ATHLETICS }, 10, 0 }, { { SKILL_SNEAK }, 5, 0 } },
                        218 },
                    { "verdathi", "Verdathi", "Marsh-born reptilians with no patience for pretense. Sturdy on land, something else entirely in water.",
                        -5, 5, 5, 5, 0, -5, -5,
                        { { { SKILL_ATHLETICS }, 10, 0 }, { { SKILL_SECURITY }, 5, 0 } },
                        219 }
};

const RaceData* FindRace(const std::string& id)
{
    for (size_t i = 0; i < G_RACES.size(); i++)
    {
        if (G_RACES[i].id == id)
        {
            return &G_RACES[i];
        }
    }
    return nullptr;
}

std::vector<SkillBonus> RollRaceSkillBonuses(const RaceData& race)
{
    std::vector<SkillBonus> bonuses;

    for (size_t r = 0; r < race.skillRules.size(); r++)
    {
        const RaceSkillRule& rule = race.skillRules[r];

        if (rule.pickCount <= 0)
        {
            for (size_t i = 0; i < rule.skillIds.size(); i++)
            {
                SkillBonus bonus;
                bonus.skillId = rule.skillIds[i];
                bonus.amount = rule.amount;
                bonus.mode = rule.mode;
                bonuses.push_back(bonus);
            }
        }
        else
        {
            std::vector<int> pool = rule.skillIds;
            int picks = rule.pickCount;
            if (picks > (int)pool.size())
            {
                picks = (int)pool.size();
            }

            for (int p = 0; p < picks; p++)
            {
                int index = GetRandomValue(0, (int)pool.size() - 1);
                SkillBonus bonus;
                bonus.skillId = pool[index];
                bonus.amount = rule.amount;
                bonus.mode = rule.mode;
                bonuses.push_back(bonus);
                pool.erase(pool.begin() + index);
            }
        }
    }

    return bonuses;
}

std::string DescribeSkillBonuses(const std::vector<SkillBonus>& bonuses)
{
    std::string text = "";
    std::string setNames = "";
    int setAmount = 0;

    for (size_t i = 0; i < bonuses.size(); i++)
    {
        std::string skillName = "Unknown";
        if (bonuses[i].skillId >= 0 && bonuses[i].skillId < (int)G_SKILL_TYPES.size())
        {
            skillName = G_SKILL_TYPES[bonuses[i].skillId].name;
        }

        if (bonuses[i].mode == SKILL_BONUS_SET)
        {
            if (!setNames.empty())
            {
                setNames += ", ";
            }
            setNames += skillName;
            setAmount = bonuses[i].amount;
        }
        else
        {
            if (!text.empty())
            {
                text += "   |   ";
            }
            text += "+" + std::to_string(bonuses[i].amount) + " " + skillName;
        }
    }

    if (!setNames.empty())
    {
        if (!text.empty())
        {
            text += "   |   ";
        }
        text += setNames + " set to " + std::to_string(setAmount); // Assumes one SET amount per race
    }

    return text;
}