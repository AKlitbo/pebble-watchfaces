// unit conversions: swatch .beats, meters to mi/km, distance formatting
#include "units.h"

#include <stdio.h>

// current swatch internet time in .beats (0..999)
int units_swatch_beats(void)
{
    time_t now = time(NULL);
    struct tm *u = gmtime(&now);
    long secs = u->tm_hour * 3600 + u->tm_min * 60 + u->tm_sec;
    return (int)(((secs + 3600) % 86400) / 86.4);  // 0..999
}

// meters to miles
float units_m_to_miles(int meters)
{
    return meters / 1609.344f;
}

// meters to kilometers
float units_m_to_km(int meters)
{
    return meters / 1000.0f;
}

// format meters as "N.N MI" or "N.N KM"
void units_format_distance(char *buffer, size_t size, int meters, bool miles)
{
    float dist = miles ? units_m_to_miles(meters) : units_m_to_km(meters);
    int whole = (int)dist;
    int tenth = (int)((dist - whole) * 10);
    snprintf(buffer, size, "%d.%d %s", whole, tenth, miles ? "MI" : "KM");
}
