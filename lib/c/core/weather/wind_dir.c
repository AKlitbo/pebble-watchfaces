/**
 * @file wind_dir.c
 * @brief The compass direction a wind is blowing from, as a bearing and as a sideways component.
 */
#include "weather/wind_dir.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

#define POINT_COUNT 16

/** @brief Whether two compass abbreviations are the same, ignoring case. */
static bool same_point(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b))
        {
            return false;
        }
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

/**
 * @brief Sine in thousandths, from a table every 15 degrees with the gaps interpolated.
 *
 * The watch has a trig lookup but this file is pure core and cannot reach it, and pulling float
 * maths in for one component would cost more than the table does. Straight lines between
 * fifteen-degree stops land within a percent of a real sine, which is finer than a caller
 * working in whole pixels or whole percent can use.
 */
static int sin_milli(int degrees)
{
    static const short table[] = {
        0, 259, 500, 707, 866, 966, 1000, 966, 866, 707, 500, 259, 0,
        -259, -500, -707, -866, -966, -1000, -966, -866, -707, -500, -259, 0,
    };

    degrees %= 360;
    if (degrees < 0)
    {
        degrees += 360;
    }

    int slot = degrees / 15;
    int into = degrees % 15;

    return table[slot] + ((table[slot + 1] - table[slot]) * into) / 15;
}

int wind_bearing(const char *dir)
{
    static const char *const points[POINT_COUNT] = {
        "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
        "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW",
    };

    if (!dir || !dir[0])
    {
        return -1;
    }

    for (int i = 0; i < POINT_COUNT; i++)
    {
        if (same_point(points[i], dir))
        {
            // 22.5 degrees a point, kept in whole degrees. the half degree it drops is far below
            // what the reading is accurate to in the first place
            return (i * 45) / 2;
        }
    }

    return -1;
}

int wind_lean(int bearing)
{
    if (bearing < 0)
    {
        return 0;
    }

    // the bearing is where the wind comes *from*, so it blows towards bearing + 180 and the
    // eastward part of that is -sin(bearing). a wind straight towards or away from the viewer
    // has no eastward part at all, which is what makes this zero at due north and due south
    return -sin_milli(bearing) / 10;
}
