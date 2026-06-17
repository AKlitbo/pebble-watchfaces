// unit conversions: swatch .beats, meters to mi/km, distance formatting
#include "units.h"

#include <stdio.h>

// current swatch internet time in .beats (0..999)
int units_swatch_beats(void)
{
    time_t now;
    uint16_t ms;

    // seconds + the ms within that second
    time_ms(&now, &ms);  
    
    struct tm *u = gmtime(&now);
    long secs = u->tm_hour * 3600 + u->tm_min * 60 + u->tm_sec;

    // work in ms: a beat is 86400 ms and boundaries fall mid-second, so flooring
    // whole seconds reads one beat behind right when the refresh timer fires
    long ms_into_day = (((secs + 3600) % 86400) * 1000L) + ms;
    return (int)((ms_into_day % 86400000L) / 86400L);  // 0..999
}

// ms until the next .beats boundary (a beat is 86400 ms), so a timer can re-arm
// to flip the @beat readout exactly when the beat rolls over
uint32_t units_ms_until_next_beat(void)
{
    time_t now;
    uint16_t ms;

    // seconds + the ms within that second
    time_ms(&now, &ms);  

    struct tm *u = gmtime(&now);
    long secs = u->tm_hour * 3600 + u->tm_min * 60 + u->tm_sec;

    // ms since BMT (UTC+1) midnight, then ms into the current 86400 ms beat
    long ms_into_day = (((secs + 3600) % 86400) * 1000L) + ms;
    long into_beat = ms_into_day % 86400L;

    // 1..86400, never 0
    return (uint32_t)(86400L - into_beat);
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

// format meters as "N.N MI" or "N.N KM", rounded to the nearest tenth
void units_format_distance(char *buffer, size_t size, int meters, bool miles)
{
    float dist = miles ? units_m_to_miles(meters) : units_m_to_km(meters);

    // round to one decimal in integer space so the carry is handled (4.96 -> 5.0)
    int tenths = (int)(dist * 10.0f + 0.5f);
    snprintf(buffer, size, "%d.%d %s", tenths / 10, tenths % 10, miles ? "MI" : "KM");
}
