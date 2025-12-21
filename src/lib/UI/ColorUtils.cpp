#include "ColorUtils.h"
#include <algorithm>

CF_Color blend(CF_Color color1, CF_Color color2, float blend_time, float current_time)
{
    // Calculate the blend factor (0.0 to 1.0)
    float t = std::clamp(current_time / blend_time, 0.0f, 1.0f);

    // Linearly interpolate between the two colors
    CF_Color result;
    result.r = color1.r + (color2.r - color1.r) * t;
    result.g = color1.g + (color2.g - color1.g) * t;
    result.b = color1.b + (color2.b - color1.b) * t;
    result.a = color1.a + (color2.a - color1.a) * t;

    return result;
}
