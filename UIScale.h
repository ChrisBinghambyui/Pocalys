#pragma once

// Multiplier derived from the current screen height against a 1080p reference, clamped so it
// never goes absurd on a tiny window or a huge monitor. Every font size and UI dimension should
// be multiplied by this instead of using a bare pixel constant.
float GetUIScale();