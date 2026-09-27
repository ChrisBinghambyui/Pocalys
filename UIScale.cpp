#include "UIScale.h"
#include <raylib.h>

float GetUIScale()
{
    float scale = (float)GetScreenHeight() / 1080.0f;
    if (scale < 0.75f)
    {
        scale = 0.75f;
    }
    if (scale > 2.5f)
    {
        scale = 2.5f;
    }
    return scale;
}