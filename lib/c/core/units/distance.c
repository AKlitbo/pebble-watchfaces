/**
 * @file distance.c
 * @brief Pure distance math implementation
 */
#include "units/distance.h"

#include <stdio.h>

const char *distance_unit(bool miles)
{
    return miles ? "MI" : "KM";
}

void distance_format_value(char *buffer, size_t size, int meters, bool miles)
{
    if (meters < 0)
    {
        meters = 0;
    }

    // tenths of the display unit in whole integer math so no float library gets linked in.
    // a tenth of a mile is 160.9344 m so scale by 625/100584 (10000/1609344 reduced) and
    // a tenth of a km is a flat 100 m. adding half the divisor rounds to the nearest tenth
    // and the miles product stays inside 32 bits below about 3400 km
    int tenths = miles ? (meters * 625 + 50292) / 100584 : (meters + 50) / 100;
    snprintf(buffer, size, "%d.%d", tenths / 10, tenths % 10);
}

void distance_format(char *buffer, size_t size, int meters, bool miles)
{
    char value[12];
    distance_format_value(value, sizeof(value), meters, miles);
    snprintf(buffer, size, "%s %s", value, distance_unit(miles));
}
