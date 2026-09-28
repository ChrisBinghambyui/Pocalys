#pragma once

// Skill level (0-100) converted into combat outcome modifiers. Landing a hit is decided by
// positioning and timing (weapon hitbox), not by these skills; skill only affects what happens
// once a hit lands. See Combat.cpp's ResolveBumpAttack for where these get applied.

// Multiplies weapon/unarmed damage. 1.5x at skill 100. Placeholder curve, tune after playtesting.
float GetSkillDamageMultiplier(int skillLevel);

// Percent chance (0-100) a landed hit is a critical. 5% base, scales to 25% at skill 100. Placeholder curve.
int GetSkillCritChancePercent(int skillLevel);

// Flat AR bonus from an armor skill, per the rulebook's armor skill table. Not wired into player
// armor calculation yet, since there's no GetPlayerArmor equivalent to GetEnemyArmor. Ready for
// when one exists.
int GetArmorSkillBonus(int skillLevel);

// Multiplies attack speed once real-time swing timing exists. Not called anywhere yet, phase one
// keeps the turn-based bump loop. Placeholder for the twin-stick conversion.
float GetSkillSpeedMultiplier(int skillLevel);