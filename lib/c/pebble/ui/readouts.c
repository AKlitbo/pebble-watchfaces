/**
 * @file readouts.c
 * @brief Text formatters shared by the faces, bound to the engine's text-slots.
 */
#include "ui/readouts.h"

#include <stdio.h>
#include <string.h>

#include "io/stores/time_store.h"
#include "io/stores/health_store.h"
#include "io/stores/weather_store.h"
#include "io/stores/location_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "system/units/units.h"
#include "text/text_case.h"

void readout_time(char *out, size_t n)
{
    uint8_t time_format = settings_u8(SETTING_TIME_FORMAT);

    // swatch .beats replaces the clock
    if (time_format == TIME_FORMAT_BEATS)
    {
        snprintf(out, n, "@%03d", units_swatch_beats());
        return;
    }

    // 24 -> 12 -> System
    bool h24 = (time_format == TIME_FORMAT_24H)
        || (time_format == TIME_FORMAT_SYSTEM && clock_is_24h_style());

    strftime(out, n, h24 ? "%H:%M" : "%I:%M", time_store_tm());
}

void readout_meridiem(char *out, size_t n)
{
    uint8_t time_format = settings_u8(SETTING_TIME_FORMAT);

    // AM/PM only makes sense on a 12-hour clock (explicit 12-hour or System when the
    // watch isn't in 24h mode). empty otherwise so it never shows on 24h/.beats
    bool h12 = (time_format == TIME_FORMAT_12H)
        || (time_format == TIME_FORMAT_SYSTEM && !clock_is_24h_style());

    if (h12)
    {
        snprintf(out, n, "%s", time_store_tm()->tm_hour < 12 ? "AM" : "PM");
    }
    else
    {
        out[0] = '\0';
    }
}

void readout_date(char *out, size_t n)
{
    strftime(out, n, settings_str(SETTING_DATE_FORMAT), time_store_tm());
    text_to_upper(out);
}

void readout_hr(char *out, size_t n)
{
    int hr = health_store_hr();
    if (hr > 0)
    {
        snprintf(out, n, "%d", hr);
    }
    else
    {
        snprintf(out, n, "--");
    }
}

void readout_steps(char *out, size_t n)
{
    uint8_t mode = settings_u8(SETTING_STEPS_MODE);
    if (mode == STEPS_MODE_MILES || mode == STEPS_MODE_KM)
    {
        units_format_distance(out, n, health_store_distance_m(), mode == STEPS_MODE_MILES);
    }
    else
    {
        snprintf(out, n, "%d", health_store_steps());
    }
}

void readout_weather_temp(char *out, size_t n)
{
    // the store hands back a whole number already in the user's unit
    // WEATHER_NO_TEMP means we have not got a reading yet
    // the unit letter goes on the end so it reads like "23C" or "73F"
    int temp = weather_store_temp();
    if (temp == WEATHER_NO_TEMP)
    {
        snprintf(out, n, "--");
        return;
    }

    snprintf(out, n, "%d%c", temp, settings_u8(SETTING_TEMPERATURE_UNIT) ? 'F' : 'C');
}

void readout_weather_cond(char *out, size_t n)
{
    // drop the night marker so "CLEAR_NIGHT" reads as "CLEAR" (the glyph uses the full token)
    const char *cond = weather_store_cond();
    snprintf(out, n, "%s", cond);

    char *night = strchr(out, '_');
    if (night)
    {
        *night = '\0';
    }

    if (out[0] == '\0')
    {
        snprintf(out, n, "--");
    }
}

void readout_lat(char *out, size_t n)
{
    const char *lat = location_store_lat();
    snprintf(out, n, "%s", lat[0] ? lat : "--");
}

void readout_lon(char *out, size_t n)
{
    const char *lon = location_store_lon();
    snprintf(out, n, "%s", lon[0] ? lon : "--");
}
