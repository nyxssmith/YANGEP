#pragma once

#include <cute.h>

/**
 * Blends between two colors based on time progression.
 *
 * @param color1 The starting color
 * @param color2 The ending color
 * @param blend_time The total time for the blend (in seconds)
 * @param current_time The current time in the blend (in seconds)
 * @return The blended color
 */
CF_Color blend(CF_Color color1, CF_Color color2, float blend_time, float current_time);
