/**
 * @file scale.c
 * @brief The clamp and segment maths a bar or gauge is built from.
 */
#include "math/scale.h"

int clamp_int(int value, int lo, int hi)
{
    if (value < lo)
    {
        return lo;
    }
    if (value > hi)
    {
        return hi;
    }
    return value;
}

int segment_width(int total, int gap, int count)
{
    if (count <= 0)
    {
        return 0;
    }
    return (total - gap * (count - 1)) / count;
}

int fraction_px(int total, int num, int den)
{
    if (den <= 0)
    {
        return 0;
    }
    return total * num / den;
}

int segments_filled(int level, int segments)
{
    if (segments <= 0)
    {
        return 0;
    }

    level = clamp_int(level, 0, 100);

    // round up so any charge lights at least one cell and 100% lights them all
    int filled = (level * segments + 99) / 100;
    return filled > segments ? segments : filled;
}

int plot_y(int y0, int height, int lo, int hi, int value)
{
    int span = hi - lo;
    if (span <= 0)
    {
        return y0 + height - 1;
    }

    // pin an out-of-window value to an edge so it never maps to a row outside the area
    value = clamp_int(value, lo, hi);
    return y0 + height - 1 - ((value - lo) * (height - 1)) / span;
}
