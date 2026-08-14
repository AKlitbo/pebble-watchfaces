/**
 * @file ops_text.c
 * @brief The value formatters behind the ops catalog.
 *
 * Every reading here has a no-data state, and all of them answer "--" for it rather than
 * printing a bogus zero, matching the shared readouts.
 */
#include "ops/ops_text.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "clock/astro.h"
#include "clock/clockstr.h"
#include "clock/date.h"
#include "clock/duration.h"
#include "clock/moon.h"
#include "clock/solar.h"
#include "io/stores/health_store.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "settings_schema.h"
#include "system/units/units.h"
#include "text/text_case.h"

// the eight phase glyphs in cycle order (0 new, 4 full, wrapping back round). the index comes
// straight from moon_glyph_index so this table only turns it into a resource id
static const uint32_t s_moon_res[8] = {
    RESOURCE_ID_ICON_MOON_0, RESOURCE_ID_ICON_MOON_1, RESOURCE_ID_ICON_MOON_2, RESOURCE_ID_ICON_MOON_3,
    RESOURCE_ID_ICON_MOON_4, RESOURCE_ID_ICON_MOON_5, RESOURCE_ID_ICON_MOON_6, RESOURCE_ID_ICON_MOON_7,
};

/**
 * @brief Write the no-reading marker.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 */
static void no_data(char *out, size_t n)
{
    snprintf(out, n, "--");
}

/**
 * @brief Write a length of time as "13H 43M".
 *
 * Every duration in the catalog goes through here, so only a real clock reading ever comes out
 * as "HH:MM". A day length written 13:43 sitting under a sunset written 20:14 reads as a third
 * time of day rather than as a span.
 *
 * @param out Output buffer.
 * @param n Buffer size.
 * @param minutes The span in minutes.
 */
static void write_duration(char *out, size_t n, int minutes)
{
    // the compact form writes its units in lower case, and this font is cut to capitals only
    duration_hm_compact(out, n, minutes);
    text_to_upper(out);
}

/**
 * @brief Today's sunrise and sunset as minutes past midnight.
 *
 * The phone sends them as "HH:MM" strings, so this is the seam where they become numbers the
 * solar math can use.
 *
 * @param rise Receives sunrise, or -1 when there is no reading.
 * @param set Receives sunset, or -1 when there is no reading.
 * @return True when both landed.
 */
static bool sun_minutes(int *rise, int *set)
{
    *rise = clockstr_minutes(weather_store_sunrise());
    *set = clockstr_minutes(weather_store_sunset());
    return *rise >= 0 && *set >= 0;
}

/** @brief The clock as minutes past midnight. */
static int now_minutes(void)
{
    const struct tm *tm = time_store_tm();
    return tm->tm_hour * 60 + tm->tm_min;
}

/**
 * @brief Whether the moon countdown is running to the full moon rather than the new one.
 *
 * Both the word and the number need the answer, so they ask the same question rather than each
 * deciding for itself and risking a "FULL IN" over a count to the new moon.
 */
static bool moon_next_is_full(void)
{
    time_t now = time(NULL);
    return moon_days_to_phase(now, true) <= moon_days_to_phase(now, false);
}

// --- system ---

void ops_text_battery(char *out, size_t n)
{
    snprintf(out, n, "%d%%", system_store_battery());
}

// --- health ---

void ops_text_calories(char *out, size_t n)
{
    int kcal = health_store_calories();
    if (kcal < 0)
    {
        no_data(out, n);
        return;
    }

    snprintf(out, n, "%d", kcal);
}

void ops_text_sleep(char *out, size_t n)
{
    int minutes = health_store_sleep_min();
    if (minutes < 0)
    {
        no_data(out, n);
        return;
    }

    write_duration(out, n, minutes);
}

void ops_text_active(char *out, size_t n)
{
    int minutes = health_store_active_min();
    if (minutes < 0)
    {
        no_data(out, n);
        return;
    }

    snprintf(out, n, "%d", minutes);
}

// --- moon ---

void ops_text_moon_pct(char *out, size_t n)
{
    snprintf(out, n, "%d%%", moon_illumination_pct(time(NULL)));
}

void ops_text_moon_phase(char *out, size_t n)
{
    snprintf(out, n, "%s", moon_phase_name(time(NULL)));
}

void ops_text_moon_next(char *out, size_t n)
{
    int days = moon_days_to_phase(time(NULL), moon_next_is_full());
    if (days == 0)
    {
        snprintf(out, n, "NOW");
        return;
    }

    snprintf(out, n, "%d %s", days, days == 1 ? "DAY" : "DAYS");
}

uint32_t ops_moon_icon(void)
{
    return s_moon_res[moon_glyph_index(time(NULL), 8)];
}

const char *ops_moon_next_label(void)
{
    return moon_next_is_full() ? "FULL IN" : "NEW IN";
}

// --- sun ---

void ops_text_sunrise(char *out, size_t n)
{
    const char *at = weather_store_sunrise();
    snprintf(out, n, "%s", at[0] ? at : "--");
}

void ops_text_sunset(char *out, size_t n)
{
    const char *at = weather_store_sunset();
    snprintf(out, n, "%s", at[0] ? at : "--");
}

void ops_text_daylight(char *out, size_t n)
{
    int rise, set;
    if (!sun_minutes(&rise, &set))
    {
        no_data(out, n);
        return;
    }

    // a sunset before the sunrise means the span crosses midnight, which happens near the poles
    int span = set - rise;
    if (span < 0)
    {
        span += 1440;
    }

    write_duration(out, n, span);
}

void ops_text_sun_next(char *out, size_t n)
{
    int rise, set;
    if (!sun_minutes(&rise, &set))
    {
        no_data(out, n);
        return;
    }

    bool is_sunrise = false;
    int minutes = solar_next_event(rise, set, now_minutes(), &is_sunrise);
    if (minutes < 0)
    {
        no_data(out, n);
        return;
    }

    write_duration(out, n, minutes);
}

const char *ops_sun_next_label(void)
{
    int rise, set;
    if (!sun_minutes(&rise, &set))
    {
        return "SOLAR";
    }

    bool is_sunrise = false;
    if (solar_next_event(rise, set, now_minutes(), &is_sunrise) < 0)
    {
        return "SOLAR";
    }

    return is_sunrise ? "DAWN IN" : "DUSK IN";
}

uint32_t ops_sun_next_icon(void)
{
    int rise, set;
    if (!sun_minutes(&rise, &set))
    {
        return RESOURCE_ID_ICON_SUN;
    }

    bool is_sunrise = false;
    if (solar_next_event(rise, set, now_minutes(), &is_sunrise) < 0)
    {
        return RESOURCE_ID_ICON_SUN;
    }

    return is_sunrise ? RESOURCE_ID_ICON_SUNRISE : RESOURCE_ID_ICON_SUNSET;
}

// --- weather ---

// the SENSORS glyphs are cut for a 24px box and an ops slot only has 14. so the
// small set is a second cut of the same art
//
// it mirrors the generated table in ui/weather/icons_table.g.h. that one is built
// from lib/ts/weather/conditions.ts and only knows the 24px names
// a new condition added over there needs a row here too. otherwise it falls
// through to the NA glyph
uint32_t ops_wx_icon(void)
{
    const char *cond = weather_store_cond();

    static const struct { const char *token; uint32_t res; } map[] = {
        {"CLEAR",       RESOURCE_ID_ICON_WX_SM_CLEAR},
        {"CLEAR_NIGHT", RESOURCE_ID_ICON_WX_SM_NIGHT_CLEAR},
        {"PCLDY",       RESOURCE_ID_ICON_WX_SM_PARTLY_CLOUDY},
        {"PCLDY_NIGHT", RESOURCE_ID_ICON_WX_SM_NIGHT_PARTLY_CLOUDY},
        {"CLDY",        RESOURCE_ID_ICON_WX_SM_CLOUDY},
        {"CLDY_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_CLOUDY},
        {"FOGGY",       RESOURCE_ID_ICON_WX_SM_FOG},
        {"FOGGY_NIGHT", RESOURCE_ID_ICON_WX_SM_NIGHT_FOG},
        {"DRZL",        RESOURCE_ID_ICON_WX_SM_DRIZZLE},
        {"DRZL_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_DRIZZLE},
        {"RAIN",        RESOURCE_ID_ICON_WX_SM_RAIN},
        {"RAIN_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_RAIN},
        {"SHWR",        RESOURCE_ID_ICON_WX_SM_SHOWERS},
        {"SHWR_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_SHOWERS},
        {"SNOW",        RESOURCE_ID_ICON_WX_SM_SNOW},
        {"SNOW_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_SNOW},
        {"SNSH",        RESOURCE_ID_ICON_WX_SM_SNOW_WIND},
        {"SNSH_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_SNOW_WIND},
        {"STRM",        RESOURCE_ID_ICON_WX_SM_THUNDERSTORM},
        {"STRM_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_THUNDERSTORM},
        // freezing drizzle and freezing rain share the sleet glyph just like the 24px set
        {"FZDZ",        RESOURCE_ID_ICON_WX_SM_SLEET},
        {"FZDZ_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_SLEET},
        {"FZRN",        RESOURCE_ID_ICON_WX_SM_SLEET},
        {"FZRN_NIGHT",  RESOURCE_ID_ICON_WX_SM_NIGHT_SLEET},
    };

    for (unsigned i = 0; i < ARRAY_LENGTH(map); i++)
    {
        if (!strcmp(cond, map[i].token))
        {
            return map[i].res;
        }
    }

    return RESOURCE_ID_ICON_WX_SM_NA;
}

void ops_text_humidity(char *out, size_t n)
{
    int pct = weather_store_humidity();
    if (pct < 0)
    {
        no_data(out, n);
        return;
    }

    snprintf(out, n, "%d%%", pct);
}

void ops_text_wind(char *out, size_t n)
{
    int kmh = weather_store_wind_kmh();
    if (kmh < 0)
    {
        no_data(out, n);
        return;
    }

    // the compass point rides along when the phone sent one, so this reads "12 NW"
    const char *dir = weather_store_wind_dir();
    if (dir[0])
    {
        snprintf(out, n, "%d %s", kmh, dir);
        return;
    }

    snprintf(out, n, "%d", kmh);
}

/**
 * @brief EPA-style risk band for a uv index. A bare number means little without it.
 *
 * The same wording the mosaic faces use, so a 7 reads HIGH wherever you see it.
 *
 * @param uv The index.
 * @return The band name.
 */
static const char *uv_risk(int uv)
{
    if (uv <= 2)  { return "LOW"; }
    if (uv <= 5)  { return "MOD"; }
    if (uv <= 7)  { return "HIGH"; }
    if (uv <= 10) { return "V.HIGH"; }
    // gridlock spells this one out, but it has a panel to do it in. here the number rides in
    // front of it in a 55px slot
    return "EXTRM";
}

void ops_text_uv(char *out, size_t n)
{
    int uv = weather_store_uv();
    if (uv < 0)
    {
        no_data(out, n);
        return;
    }

    // the top two bands run long enough to drop the readout onto its smaller font, which is
    // what that fallback is for. they are also the two worth reading properly
    snprintf(out, n, "%d %s", uv, uv_risk(uv));
}

void ops_text_hilo(char *out, size_t n)
{
    int hi = weather_store_temp_max();
    int lo = weather_store_temp_min();
    if (hi == WEATHER_NO_TEMP || lo == WEATHER_NO_TEMP)
    {
        no_data(out, n);
        return;
    }

    snprintf(out, n, "%d/%d", hi, lo);
}

// --- calendar ---

void ops_text_julian(char *out, size_t n)
{
    // the shared helper keeps the hundredths so it can stay in 32 bits, and the whole day is
    // what a stardate-style readout wants
    snprintf(out, n, "%d", (int)(astro_jd_centi(time(NULL)) / 100));
}

void ops_text_day_of_year(char *out, size_t n)
{
    snprintf(out, n, "%d", date_day_of_year(time_store_tm()->tm_yday));
}

void ops_text_week(char *out, size_t n)
{
    const struct tm *tm = time_store_tm();
    snprintf(out, n, "%d", date_iso_week(tm->tm_year + 1900, tm->tm_yday, tm->tm_wday));
}

// --- other clocks ---

void ops_text_epoch(char *out, size_t n)
{
    // off the time store rather than time(NULL) so it tracks the same tick as everything else and
    // freezes with the dev clock. mktime wants a writable tm so it gets a copy
    struct tm now = *time_store_tm();
    snprintf(out, n, "%ld", (long)mktime(&now));
}

void ops_text_beats(char *out, size_t n)
{
    // three digits always, so the row does not jump width as the count rolls over
    snprintf(out, n, "@%03d", units_swatch_beats());
}

void ops_text_zone_1(char *out, size_t n)
{
    // off the time store like the rest of the face, so it tracks the same tick and freezes with
    // the dev clock. mktime turns the local reading back into UTC, then the offset and gmtime
    // give the other zone's wall clock rather than this one's
    struct tm here = *time_store_tm();
    time_t at = mktime(&here) + (lcars_zone_1_offset_minutes() * 60);
    struct tm *there = gmtime(&at);
    if (!there)
    {
        no_data(out, n);
        return;
    }

    // 24 hour, matching DAWN and DUSK. an AM or PM would not fit beside the glyph anyway
    snprintf(out, n, "%02d:%02d", there->tm_hour, there->tm_min);
}

const char *ops_zone_1_label(void)
{
    static char label[10];

    // the setting holds "offset,City, Region, CC" so the word stops at the first comma, which is
    // the city on its own. nine characters is all the holder box can hold
    const char *name = lcars_zone_1_name();
    size_t i = 0;
    while (i < sizeof(label) - 1 && name[i] && name[i] != ',')
    {
        label[i] = name[i];
        i++;
    }
    label[i] = '\0';

    text_to_upper(label);
    return label[0] ? label : "ZONE 1";
}
