/**
 * @file ops.c
 * @brief The ops catalog: one row per readout a slot can show.
 *
 * Indexed by OpsId so a row can never drift from the number that is persisted for it. The words
 * are the LCARS vocabulary the rest of the frame already speaks (SENSORS, STARDATE), which is why
 * the heart rate reads VITALS rather than "heart rate".
 */
#include "ops/ops.h"

#include "ops/ops_text.h"
#include "ui/readouts.h"

// a word wider than the holder box gets clipped rather than wrapped, so these stay inside the
// nine characters LBL_OPS_A can hold at Antonio 10
static const OpsReadout s_catalog[OPS_COUNT] = {
    [OPS_HEART]       = {.label = "VITALS",    .icon = RESOURCE_ID_ICON_HEART,       .text = readout_hr},
    [OPS_STEPS]       = {.label = "TRAVERSAL", .icon = RESOURCE_ID_ICON_FEET,        .text = readout_steps},
    [OPS_BATTERY]     = {.label = "POWER",     .icon = RESOURCE_ID_ICON_POWER,       .text = ops_text_battery},
    [OPS_CALORIES]    = {.label = "METABOLIC", .icon = RESOURCE_ID_ICON_CALORIES,    .text = ops_text_calories},
    [OPS_SLEEP]       = {.label = "REGEN",     .icon = RESOURCE_ID_ICON_SLEEP,       .text = ops_text_sleep},
    [OPS_ACTIVE]      = {.label = "EXERTION",  .icon = RESOURCE_ID_ICON_ACTIVE,      .text = ops_text_active},

    [OPS_MOON_PCT]    = {.label = "LUNAR",     .icon_for = ops_moon_icon,            .text = ops_text_moon_pct},
    [OPS_MOON_PHASE]  = {.label = "LUNAR",     .icon_for = ops_moon_icon,            .text = ops_text_moon_phase},
    [OPS_MOON_NEXT]   = {.label_for = ops_moon_next_label,
                         .icon_for = ops_moon_icon,                                  .text = ops_text_moon_next},

    [OPS_SUNRISE]     = {.label = "DAWN",      .icon = RESOURCE_ID_ICON_SUNRISE,     .text = ops_text_sunrise},
    [OPS_SUNSET]      = {.label = "DUSK",      .icon = RESOURCE_ID_ICON_SUNSET,      .text = ops_text_sunset},
    [OPS_DAYLIGHT]    = {.label = "DAYLIGHT",  .icon = RESOURCE_ID_ICON_SUN,         .text = ops_text_daylight},
    [OPS_SUN_NEXT]    = {.label_for = ops_sun_next_label,
                         .icon_for = ops_sun_next_icon,                              .text = ops_text_sun_next},

    [OPS_HUMIDITY]    = {.label = "ATMOS",     .icon = RESOURCE_ID_ICON_HUMIDITY,    .text = ops_text_humidity},
    [OPS_WIND]        = {.label = "AIRFLOW",   .icon = RESOURCE_ID_ICON_WIND,        .text = ops_text_wind},
    [OPS_UV]          = {.label = "UV INDEX",  .icon = RESOURCE_ID_ICON_UV,          .text = ops_text_uv},
    // the weather block's thermometer is cut 13x17 for its own taller box, so this row takes a
    // shorter one that fits the 14x14 ops glyph without losing its bulb
    [OPS_HILO]        = {.label = "RANGE",     .icon = RESOURCE_ID_ICON_THERMOMETER_SM, .text = ops_text_hilo},

    // the three calendar counts share one glyph. they are all a way of numbering a day
    [OPS_JULIAN]      = {.label = "JULIAN",    .icon = RESOURCE_ID_ICON_CALENDAR,    .text = ops_text_julian},
    [OPS_DAY_OF_YEAR] = {.label = "SOL",       .icon = RESOURCE_ID_ICON_CALENDAR,    .text = ops_text_day_of_year},
    [OPS_WEEK]        = {.label = "CYCLE",     .icon = RESOURCE_ID_ICON_CALENDAR,    .text = ops_text_week},

    // the row stays in the table so the empty pick resolves like any other
    [OPS_EMPTY]       = {0},

    // the same two readings the SENSORS block shows but sized to fit an ops row
    [OPS_WEATHER_TEMP] = {.label = "THERMAL", .icon = RESOURCE_ID_ICON_THERMOMETER_SM,
                          .text = readout_weather_temp},
    [OPS_WEATHER_COND] = {.label = "SKY",     .icon_for = ops_wx_icon,
                          .text = readout_weather_cond},

    // the tall weather block. one bar over a big condition glyph and a large
    // temperature filling both rows of its column
    // the row here carries only the word. layout draws the rest from its own slots
    // rather than through this table
    [OPS_SENSORS]      = {.label = "SENSORS", .tall = true},

    // three other ways of saying what time it is
    // epoch takes no glyph on purpose: ten digits only fit once the row hands its icon space
    // back to the value, which layout does for any row with no glyph
    [OPS_EPOCH]        = {.label = "EPOCH",   .text = ops_text_epoch},
    [OPS_BEATS]        = {.label = "BEATS",   .icon = RESOURCE_ID_ICON_GLOBE,
                          .text = ops_text_beats},
    [OPS_ZONE_1]       = {.label_for = ops_zone_1_label, .icon = RESOURCE_ID_ICON_GLOBE,
                          .text = ops_text_zone_1},
};

const OpsReadout *ops_entry(uint8_t id)
{
    // a blob written by a later build can carry a number this one has no row for, so fall back
    // to the empty row rather than reading off the end of the table
    if (id >= OPS_COUNT)
    {
        return &s_catalog[OPS_EMPTY];
    }

    return &s_catalog[id];
}

const char *ops_label(const OpsReadout *entry)
{
    if (entry->label_for)
    {
        return entry->label_for();
    }

    return entry->label ? entry->label : "";
}

uint32_t ops_icon(const OpsReadout *entry)
{
    if (entry->icon_for)
    {
        return entry->icon_for();
    }

    return entry->icon;
}

void ops_text(const OpsReadout *entry, char *out, size_t n)
{
    if (!entry->text)
    {
        out[0] = '\0';
        return;
    }

    entry->text(out, n);
}

bool ops_is_tall(uint8_t id)
{
    return ops_entry(id)->tall;
}
