#include "FeatData.h"
#include "Player.h"

std::vector<Feat> G_FEATS = {
    // ==========================================
    // MARTIAL FEATS
    // ==========================================
    { 0, "Bladestorm", FEAT_MARTIAL, "After a successful Long Blade hit, make one additional attack at +20 to roll. Once per combat, activated ability.", false, false, {}, {{5, 50}}, {} },
    { 1, "Shield Wall", FEAT_MARTIAL, "On a successful Block, your next attack against that attacker is made with -10 on the roll.", false, false, {}, {{0, 50}}, {} },
    { 2, "Bone-Crusher", FEAT_MARTIAL, "Critical blunt hits (natural 1-2) apply Prone to the target and push them back 10 ft.", false, false, {}, {{4, 50}}, {} },
    { 5, "Axe-Master", FEAT_MARTIAL, "Your Axe attacks bypass 1 point of the target's AR.", false, false, {}, {{6, 60}}, {} },
    { 6, "Calculated Thrust", FEAT_MARTIAL, "If you move away from a target and then attack with a spear, deal +1d6 bonus damage.", false, false, {}, {{7, 50}}, {} },
    { 7, "Dead Eye", FEAT_MARTIAL, "No FP cost for ranged attacks. The ranged critical hit window increases by 2.", false, false, {}, {{23, 60}}, {} },
    { 8, "Unbroken", FEAT_MARTIAL, "When you hit 0 FP, you have one additional round before Staggered applies.", false, false, {{ATTRIBUTE_END, 60}}, {}, {} },
    { 9, "Dual Strike", FEAT_MARTIAL, "Off-hand attack penalty reduced from +20 to +10 to roll.", false, false, {}, {{22, 60}}, {} },
    { 10, "Sure-Footed Grip", FEAT_MARTIAL, "When wielding a one-handed weapon with a shield, you no longer suffer the +5 attack roll penalty.", false, false, {}, {{0, 50}}, {} },
    { 11, "Armor Piercer", FEAT_MARTIAL, "Once per floor, make an attack Armor-Piercing to ignore target's AR entirely.", false, false, {{ATTRIBUTE_STR, 60}}, {{-1, 70}}, {} },
    { 12, "Brutal Momentum", FEAT_MARTIAL, "Once per floor, make one attack that completely ignores AR (separate second use).", false, false, {{ATTRIBUTE_STR, 70}}, {}, {11} },
    { 13, "Relentless Impact", FEAT_MARTIAL, "Your weapon attacks always deal a minimum of 1 damage on a hit, regardless of other modifiers or AR.", false, false, {{ATTRIBUTE_STR, 55}}, {{-1, 60}}, {} },
    { 14, "Evasive Instinct", FEAT_MARTIAL, "When targeted by an attack, make an AGI roll with a +50 penalty. On a success, you take no damage from that attack.", false, false, {{ATTRIBUTE_AGI, 65}}, {{8, 60}}, {} },
    { 15, "Spirit Bane", FEAT_MARTIAL, "Non-silver/non-magical weapons still deal half damage to incorporeal entities instead of none.", false, false, {}, {{14, 40}}, {} },
    { 16, "Favored Enemy", FEAT_MARTIAL, "Gain -10 to attack rolls and +1 damage against a chosen creature type, which can be changed on descending to a new floor.", false, true, {}, {{-1, 50}}, {} },
    { 17, "Favored Weapon", FEAT_MARTIAL, "+5 attack rolls, +1 damage, and expanded crit threshold with a specific weapon type, which can be changed on descending to a new floor.", false, false, {}, {{-1, 50}}, {} },
    { 18, "Armor Breach", FEAT_MARTIAL, "Attempt a Hard weapon roll to permanently reduce target's AR by 2 for the combat.", false, false, {}, {{-1, 50}}, {} },
    { 19, "Sentinel", FEAT_MARTIAL, "Reaction strikes against disengaging enemies have no difficulty penalty and halt movement.", false, false, {}, {{-1, 60}, {0, 40}}, {} },
    { 20, "Duelist's Focus", FEAT_MARTIAL, "Consecutive attacks against the same target gain escalating bonuses of -5 up to -15.", false, false, {}, {{-1, 50}}, {} },
    { 21, "Sweeping Strike", FEAT_MARTIAL, "Slashing and crushing attacks automatically hit adjacent targets for 25% damage.", false, false, {{ATTRIBUTE_STR, 50}}, {{-1, 60}}, {} },
    { 22, "Battle Dancer", FEAT_MARTIAL, "Gain a stacking -5 to attack when targetting a new enemy each turn. This bonus is lost after two turns of not attacking a new target.", false, false, {{ATTRIBUTE_AGI, 60}}, {{20, 50}}, {} },
    { 23, "Iron Grip", FEAT_MARTIAL, "Advantage on grappling Athletics rolls. Foes need 2 actions to break free.", false, false, {{ATTRIBUTE_STR, 50}}, {{8, 50}}, {} },
    { 24, "Paced Combat", FEAT_MARTIAL, "Regain 5% of max FP at the start of each turn.", false, false, {{ATTRIBUTE_AGI, 60}}, {{8, 60}}, {} },

    // ==========================================
    // DEFENSE FEATS
    // ==========================================
    { 25, "Second Skin (Light)", FEAT_DEFENSE, "+1 AR in Light Armor. Light armor imposes no penalty to sneaking.", false, false, {}, {{21, 60}}, {} },
    { 26, "Second Skin (Medium)", FEAT_DEFENSE, "+1 AR in Medium Armor. +10% carry weight.", false, false, {}, {{2, 60}}, {} },
    { 27, "Second Skin (Heavy)", FEAT_DEFENSE, "+1 AR in Heavy Armor. Halve FP penalties from Heavy Armor.", false, false, {}, {{3, 60}}, {} },
    { 28, "Shadow-Step", FEAT_DEFENSE, "Enter stealth mode without spending a turn.", false, false, {}, {{17, 60}, {19, 60}}, {} },
    { 29, "Reactive Parry", FEAT_DEFENSE, "Block without a shield using your weapon at +10 to the roll.", false, false, {}, {{0, 70}}, {} },
    { 30, "Practiced Guard", FEAT_DEFENSE, "Recover Block reaction if attacker's roll would have missed anyway.", false, false, {}, {{0, 50}}, {} },
    { 31, "Watchful Stance", FEAT_DEFENSE, "Treat enemies within 5m as Engaged for reaction strikes when they move, even if they're not leaving melee range.", false, false, {}, {{-1, 60}}, {19} },

    // ==========================================
    // UTILITY & ADVANCEMENT FEATS
    // ==========================================
    { 34, "Armor Rend", FEAT_UTILITY, "Attacks reduce the target's effective AR by 2 for damage calculation.", false, false, {{ATTRIBUTE_STR, 55}}, {{-1, 50}}, {} },
    { 35, "Killing Edge", FEAT_UTILITY, "Expand critical success threshold by 1.", false, true, {}, {{-1, 50}}, {} },
    { 36, "Attribute Training", FEAT_UTILITY, "Increase 2 Attributes by 5 each, or 1 Attribute by 10.", false, true, {}, {}, {} },
    { 37, "Robust Vitality", FEAT_UTILITY, "Retroactively increase Max HP by 5 per Level, and gain +5 HP per future level.", false, true, {{ATTRIBUTE_END, 40}}, {}, {} },
    { 38, "Iron Endurance", FEAT_UTILITY, "Retroactively increase Max FP by 5 per Level, and gain +5 FP per future level.", false, true, {{ATTRIBUTE_STR, 40}, {ATTRIBUTE_END, 40}}, {}, {} },
    { 39, "Arcane Reserves", FEAT_UTILITY, "Retroactively increase Max MP by 5 per Level, and gain +5 MP per future level.", false, true, {{ATTRIBUTE_INT, 40}}, {}, {} },
    { 40, "Focused Training", FEAT_UTILITY, "Selected skill levels twice as fast. Can be changed on descending to a new floor.", false, true, {}, {{-1, 30}}, {} },
    { 41, "Steady Footing", FEAT_UTILITY, "Cannot be knocked Prone by physical attacks or non-magical movement.", false, false, {}, {{0, 40}}, {} },
    { 42, "Potion Slinger", FEAT_UTILITY, "Throw potions up to 30m with a 2m area of effect.", false, false, {}, {{23, 50}, {16, 40}}, {} },

    // ==========================================
    // MAGIC FEATS
    // ==========================================
    { 43, "Focused Casting", FEAT_MAGIC, "Spells from chosen school cost 15% less MP (min 1).", false, true, {}, {{-1, 50}}, {} },
    { 44, "Spellblade Focus", FEAT_MAGIC, "Succesful hits with a weapon of the chosen type automatically cast a spell of the chosen school. Can be changed on descending to a new floor.", false, false, {}, {{-1, 50}}, {} },
    { 45, "Overchannel", FEAT_MAGIC, "Once per floor, cast a Journeyman spell at 0 MP cost.", false, false, {}, {{-1, 80}}, {} },
    { 46, "Steady Recovery", FEAT_MAGIC, "Recover 2x MP on short rests.", false, false, {{ATTRIBUTE_WIL, 50}}, {}, {} },
    { 47, "School Specialist", FEAT_MAGIC, "Spells of the chosen school can land critical hits.", false, false, {}, {{-1, 70}}, {} },
    { 48, "Twin Spells", FEAT_MAGIC, "Once per floor, cast the same spell in the chosen school twice simultaneously.", false, false, {}, {{-1, 80}}, {} },
    { 49, "Spell Shield", FEAT_MAGIC, "Spend 5 MP as a reaction to reduce incoming magic damage by 1d6.", false, false, {}, {{11, 50}}, {} },
    { 50, "Soul Sight", FEAT_MAGIC, "Passively detect the presence of souls within 30m.", false, false, {}, {{14, 50}}, {} },
    { 51, "Battle Healer", FEAT_MAGIC, "Restoration spells heal bonus HP equal to your restoration skill/5.", false, false, {}, {{15, 60}}, {} },
    { 52, "Second Chance Casting", FEAT_MAGIC, "Once per floor, turn a failed spell cast into a success.", false, false, {}, {{-1, 75}}, {} },
    { 53, "Blood Sacrifice", FEAT_MAGIC, "Once per floor, sacrifice 10% HP to restore the same amount of MP.", false, false, {{ATTRIBUTE_END, 40}}, {}, {} },
    { 55, "Dual Element", FEAT_MAGIC, "Once per floor, imbue your next Elementalism spell damage with an additionnal element type.", false, false, {}, {{10, 70}}, {} },
    { 56, "Eternal Servitude", FEAT_MAGIC, "Restore a summon to full HP for half its casting cost.", false, false, {}, {{13, 60}}, {} },
    { 57, "False Presence", FEAT_MAGIC, "Create a sensory distraction within 15m at no MP cost; Illusion spells last 1 extra round.", false, false, {}, {{12, 60}}, {} },
    { 58, "Arcane Recovery", FEAT_MAGIC, "Meditate for 10 turns once per long rest to recover MP equal to your WIL.", false, false, {}, {{-1, 60}}, {} },
    { 60, "Counter-Summon", FEAT_MAGIC, "Banish an enemy's summon as it arrives by spending matching MP.", false, false, {}, {{13, 60}}, {} },
    { 61, "Spellsmith", FEAT_MAGIC, "Reduce crafting MP costs for spells in chosen school.", false, true, {}, {{-1, 75}}, {} },
    { 62, "Stable Mind", FEAT_MAGIC, "Maintain concentration on 2 spells at once.", false, false, {}, {{-1, 80}}, {} },

    // ==========================================
    // STEALTH & SOCIAL FEATS
    // ==========================================
    { 63, "Vanish", FEAT_STEALTH_SOCIAL, "Advantage on attack rolls when attacking while undetected.", false, false, {}, {{19, 60}}, {} },
    { 66, "Fence's Eye", FEAT_STEALTH_SOCIAL, "Advantage on identifying items; get 10% better sell prices.", false, false, {}, {{24, 50}}, {} },
    { 67, "Poison Crafter", FEAT_STEALTH_SOCIAL, "Poisons deal +25% damage and last one additional round.", false, false, {}, {{16, 50}}, {} },
    { 68, "Stone-Faced", FEAT_STEALTH_SOCIAL, "Illusion and charm effects against you are made with disadvantage.", false, false, {{ATTRIBUTE_WIL, 60}}, {{25, 60}}, {} },
    { 69, "Alchemical Arm", FEAT_STEALTH_SOCIAL, "Thrown potions gain +25% damage.", false, false, {}, {{16, 60}}, {} },
    { 70, "Enhanced Potency", FEAT_STEALTH_SOCIAL, "Weapon poisons last 2 hits (3 hits with Alchemy 80+).", false, false, {}, {{16, 60}}, {67} },
    { 72, "Snake Oil Salesman", FEAT_STEALTH_SOCIAL, "Sell items to uninterested merchants with a 10% penalty.", false, false, {}, {{25, 75}}, {} },
    { 73, "Careful Hands", FEAT_STEALTH_SOCIAL, "Security rolls on traps are one difficulty lower; first failure doesn't trigger trap.", false, false, {}, {{18, 50}}, {} },
    { 74, "Potionwright", FEAT_STEALTH_SOCIAL, "Throwable potions deal an additional 25% damage; thrown and consumable potions can be used interchangably.", false, false, {}, {{16, 60}}, {69} },

    // ==========================================
    // BOONS (Gained every 5 levels)
    // ==========================================
    { 100, "Bonus Feats", BOON_UNIVERSAL, "Take 2 Feats of your choice instead of a Boon.", true, true, {}, {}, {} },
    { 101, "Weaponmaster", BOON_MARTIAL, "Crits under Skill/7 deal max damage and Stagger.", true, false, {}, {{5, 80}}, {0} },
    { 102, "Unstoppable", BOON_MARTIAL, "Cannot be Knocked Down while above half HP. Being staggered doesn't grant enemy advantage.", true, false, {}, {{3, 75}}, {27} },
    { 103, "Arcane Blade", BOON_MARTIAL, "Spend up to 10 MP once per combat to deal twice the same amount as elemental bonus damage on your next succesful offensive spell.", true, false, {}, {{5, 70}}, {44} },
    { 104, "Death from Afar", BOON_MARTIAL, "Ranged attacks cannot be blocked.", true, false, {}, {{23, 80}}, {7} },
    { 105, "Bloodrush", BOON_MARTIAL, "Once per floor, regain 50% of your max FP and gain advantage on your next attack roll.", true, false, {{ATTRIBUTE_END, 70}}, {}, {4} },
    { 106, "Archmage's Will", BOON_ARCANE, "Cast one Master-rank spell at no MP cost once per long rest.", true, false, {}, {{-1, 95}}, {47} },
    { 107, "Ritual Caster", BOON_ARCANE, "Once per floor, spend 10 turns to cast an Expert-rank spell without spending MP.", true, false, {}, {{-1, 70}}, {45} },
    { 108, "Soul Eater", BOON_ARCANE, "Absorb slain creature souls automatically without a gem to restore MP.", true, false, {}, {{14, 80}}, {50} },
    { 109, "Living Enchantment", BOON_ARCANE, "Bind one Adept-rank constant effect directly to your body permanently.", true, false, {}, {{9, 85}}, {47} },
    { 110, "Selective Casting", BOON_ARCANE, "Exclude allies or specific targets from Area of Effect spells.", true, false, {}, {{-1, 85}}, {48, 47} },
    { 111, "Bound Legion", BOON_ARCANE, "Maintain one additional summoned entity simultaneously.", true, false, {}, {{13, 80}}, {} },
    { 112, "Phantom", BOON_STEALTH_SOCIAL, "Hitting a target while undetected applies Blind to that enney for 1 turn.", true, false, {}, {{19, 85}}, {63, 28} },
    { 113, "Master Poisoner", BOON_STEALTH_SOCIAL, "Poisons affect resistant targets and last 2 additional hits.", true, false, {}, {{16, 85}}, {67, 70} },
    { 116, "Poisonblooded", BOON_STEALTH_SOCIAL, "Immune to low/medium poisons; advantage against high/extreme poisons.", true, false, {}, {{16, 90}}, {67, 70} },
    { 117, "Nearly Immortal", BOON_DEFENSIVE, "Retroactively increase Max HP by an additional 5 per Level, and gain +5 HP per future level.", true, false, {{ATTRIBUTE_END, 80}}, {}, {8, 37} },
    { 118, "Living Fortress", BOON_DEFENSIVE, "+2 effective AR against first attack each round; immune to non-magical forced movement.", true, false, {}, {{3, 85}}, {27, 102} },
    { 121, "Enduring Will", BOON_DEFENSIVE, "Immune to negative mental conditions.", true, false, {{ATTRIBUTE_WIL, 80}}, {}, {68} },
    { 123, "Master Artificer", BOON_EXPLORATION_CRAFT, "Craft Pristine items and repair broken items with basic kits.", true, false, {{ATTRIBUTE_INT, 70}}, {{1, 80}, {9, 70}}, {} },
    { 124, "Alchemical Genius", BOON_EXPLORATION_CRAFT, "When you brew a potion, roll twice and use the better score (does not count as advantage). Strong successes (rolling 20+ under skill) now produce potions at two magnitudes higher instead of one. Additionally, whenever you brew a potion, you brew that many plus one. ", true, false, {}, {{16, 90}}, {70} },
    { 125, "Grand Enchanter", BOON_EXPLORATION_CRAFT, "Soul gems give 2x charges; automatically identify magic items by touch.", true, false, {{ATTRIBUTE_INT, 85}}, {{9, 90}}, {109} },
    { 126, "Loremaster", BOON_EXPLORATION_CRAFT, "Cast spells from any school without needing them as Major/Minor skills.", true, false, {}, {{-1, 80}}, {} }
};

bool canPlayerTakeFeat(const Player& player, int featId)
{
    const Feat* targetFeat = nullptr;
    for (const auto& feat : G_FEATS) {
        if (feat.id == featId) {
            targetFeat = &feat;
            break;
        }
    }

    if (!targetFeat) return false;

    // 1. If not repeatable, check if player already has it
    if (!targetFeat->isRepeatable) {
        if (targetFeat->isBoon) {
            for (int boonId : player.activeBoons) {
                if (boonId == featId) return false;
            }
        }
        else {
            for (int fId : player.activeFeats) {
                if (fId == featId) return false;
            }
        }
    }

    // 2. Check Attribute prerequisites
    for (const auto& req : targetFeat->attrPrereqs) {
        if (player.getAttributeValue(req.attr) < req.minVal) {
            return false;
        }
    }

    // 3. Check Skill prerequisites
    for (const auto& req : targetFeat->skillPrereqs) {
        if (req.skillId == -1) { // -1 means "Any combat/magic skill"
            bool metAny = false;
            for (const auto& skillState : player.skills) {
                if (skillState.level >= req.minLevel) {
                    metAny = true;
                    break;
                }
            }
            if (!metAny) return false;
        }
        else {
            if (req.skillId >= player.skills.size() || player.skills[req.skillId].level < req.minLevel) {
                return false;
            }
        }
    }

    // 4. Check Feat prerequisites
    for (int reqFeatId : targetFeat->requiredFeatIds) {
        bool hasFeat = false;
        for (int fId : player.activeFeats) {
            if (fId == reqFeatId) { hasFeat = true; break; }
        }
        if (!hasFeat) {
            for (int bId : player.activeBoons) {
                if (bId == reqFeatId) { hasFeat = true; break; }
            }
        }
        if (!hasFeat) return false;
    }

    return true;
}