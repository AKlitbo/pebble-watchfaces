/**
 * @file distance.c
 * @brief pure distance math implementation
 */
#include "distance.h"

#include <stdio.h>

float distance_m_to_miles(int meters)
{
    return meters / 1609.344f;
}

float distance_m_to_km(int meters)
{
    return meters / 1000.0f;
}

void distance_format(char *buffer, size_t size, int meters, bool miles)
{
    float dist = miles ? distance_m_to_miles(meters) : distance_m_to_km(meters);

    // round to one decimal in integer space so the carry is handled (4.96 -> 5.0)
    int tenths = (int)(dist * 10.0f + 0.5f);
    snprintf(buffer, size, "%d.%d %s", tenths / 10, tenths % 10, miles ? "MI" : "KM");
}
