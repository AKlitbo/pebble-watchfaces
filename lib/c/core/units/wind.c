/**
 * @file wind.c
 * @brief Pure wind-speed maths implementation
 */
#include "units/wind.h"

int wind_from_kmh(int kmh, WindUnit unit)
{
    switch (unit)
    {
        case WIND_UNIT_MPH: return (kmh * 621) / 1000; // mph
        case WIND_UNIT_KTS: return (kmh * 540) / 1000; // kts
        case WIND_UNIT_MS:  return (kmh * 278) / 1000; // m/s
        default:            return kmh;                // km/h
    }
}

const char *wind_unit_label(WindUnit unit)
{
    switch (unit)
    {
        case WIND_UNIT_MPH: return "MPH";
        case WIND_UNIT_KTS: return "KTS";
        case WIND_UNIT_MS:  return "M/S";
        default:            return "KM/H";
    }
}
