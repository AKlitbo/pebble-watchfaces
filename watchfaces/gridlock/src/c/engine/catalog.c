/**
 * @file catalog.c
 * @brief The module catalog. Each module builds its readout from the feature hubs and
 * shows "--" while a reading is still empty (e.g. a health number with no data yet, or
 * weather we have not fetched).
 * @ingroup gridlock_engine
 */
#include "engine/catalog.h"
#include "draw/common.h"
#include "draw/icons.h"
#include "modules/system/system_battery.h"
#include "modules/system/system_connection.h"
#include "modules/health/health_hr_graph.h"
#include "modules/time/time_moon.h"
#include "modules/system/system_quiet.h"
#include "modules/system/system_status.h"
#include "modules/weather/weather_feels_like.h"
#include "modules/weather/weather_pressure.h"
#include "modules/weather/weather_dew_point.h"
#include "modules/weather/weather_sun.h"
#include "modules/weather/weather_temperature.h"
#include "modules/health/health_heartrate.h"
#include "modules/health/health_steps.h"
#include "modules/health/health_distance.h"
#include "modules/health/health_calories.h"
#include "modules/health/health_sleep.h"
#include "modules/health/health_activity.h"
#include "modules/weather/weather_humidity.h"
#include "modules/weather/weather_wind.h"
#include "modules/weather/weather_uv.h"
#include "modules/weather/weather_hilo.h"
#include "modules/weather/weather_precip.h"
#include "modules/weather/weather_daylight.h"
#include "modules/weather/weather_conditions.h"
#include "modules/weather/weather_current.h"
#include "modules/weather/weather_now.h"
#include "modules/weather/weather_solar.h"
#include "modules/weather/weather_forecast_hourly.h"
#include "modules/weather/weather_forecast_daily.h"
#include "modules/stock/stock_quote.h"
#include "modules/stock/stock_watchlist.h"
#include "modules/time/time_beats.h"
#include "modules/time/time_date.h"
#include "modules/time/time_clock.h"
#include "modules/time/time_zone.h"
#include "modules/time/time_analog.h"
#include "modules/composite/composite_datetime.h"
#include "modules/health/health_steps_graph.h"
#include "modules/calendar/calendar_countdown.h"
#include "modules/calendar/calendar_agenda.h"
#include "modules/calendar/calendar_freebusy.h"
#include "modules/time/time_weeknum.h"
#include "modules/time/time_dayofyear.h"
#include "modules/time/time_epoch.h"
#include "modules/time/time_yearend.h"
#include "modules/time/time_weekday_dots.h"
#include "modules/time/time_moon_countdown.h"
#include "modules/time/time_month.h"
#include "modules/time/time_julian.h"
#include "modules/time/time_weeksleft.h"
#include "modules/time/time_bignum.h"

#include <string.h>

#include "ui/fonts.h"
#include "system/units/units.h"
#include "ui/weather/icons.h"
#include "text/number_format.h"
#include "text/text_case.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "settings_schema.h"

// --- catalog ---
// .sizes is the hand-picked set of allowed sizes. keep it in step with
// src/pkjs/config.js. single-value readouts stick to the half sizes (1x2/2x2).
// a lonely "80%" in a full-width panel just looks like a mistake. only the time
// bar earns full width

static const ModuleDef s_mod_empty_def = {.label = "EMPTY", .sizes = 0};

const ModuleDef *module_def(ModuleType type)
{
    switch (type)
    {
        case MOD_SYSTEM_BATTERY: return &mod_system_battery_def;
        case MOD_WEATHER_TEMPERATURE: return &mod_weather_temperature_def;
        case MOD_HEALTH_HEARTRATE: return &mod_health_heartrate_def;
        case MOD_HEALTH_STEPS: return &mod_health_steps_def;
        case MOD_HEALTH_DISTANCE: return &mod_health_distance_def;
        case MOD_HEALTH_CALORIES: return &mod_health_calories_def;
        case MOD_HEALTH_SLEEP: return &mod_health_sleep_def;
        case MOD_HEALTH_ACTIVITY: return &mod_health_activity_def;
        case MOD_WEATHER_HUMIDITY: return &mod_weather_humidity_def;
        case MOD_WEATHER_WIND: return &mod_weather_wind_def;
        case MOD_WEATHER_SUNRISE: return &mod_weather_sunrise_def;
        case MOD_WEATHER_SUNSET: return &mod_weather_sunset_def;
        case MOD_COMPOSITE_DATETIME: return &mod_composite_datetime_def;
        case MOD_WEATHER_CURRENT: return &mod_weather_current_def;
        case MOD_WEATHER_DAYLIGHT: return &mod_weather_daylight_def;
        case MOD_WEATHER_CONDITIONS: return &mod_weather_conditions_def;
        case MOD_TIME_BEATS: return &mod_time_beats_def;
        case MOD_TIME_DATE: return &mod_time_date_def;
        case MOD_TIME_CLOCK: return &mod_time_clock_def;
        case MOD_TIME_ZONE_1: return &mod_time_zone_1_def;
        case MOD_TIME_ANALOG: return &mod_time_analog_def;
        case MOD_WEATHER_UV: return &mod_weather_uv_def;
        case MOD_WEATHER_HILO: return &mod_weather_hilo_def;
        case MOD_WEATHER_PRECIP: return &mod_weather_precip_def;
        case MOD_WEATHER_FORECAST_HOURLY: return &mod_weather_forecast_hourly_def;
        case MOD_WEATHER_FORECAST_DAILY: return &mod_weather_forecast_daily_def;
        case MOD_WEATHER_NOW: return &mod_weather_now_def;
        case MOD_WEATHER_SOLAR: return &mod_weather_solar_def;
        case MOD_STOCK_QUOTE: return &mod_stock_quote_def;
        case MOD_STOCK_WATCHLIST: return &mod_stock_watchlist_def;
        case MOD_SYSTEM_CONNECTION: return &mod_system_connection_def;
        case MOD_HEALTH_HR_GRAPH: return &mod_health_hr_graph_def;
        case MOD_TIME_MOON: return &mod_time_moon_def;
        case MOD_SYSTEM_QUIET: return &mod_system_quiet_def;
        case MOD_WEATHER_FEELS_LIKE: return &mod_weather_feels_like_def;
        case MOD_WEATHER_PRESSURE: return &mod_weather_pressure_def;
        case MOD_WEATHER_DEW_POINT: return &mod_weather_dew_point_def;
        case MOD_HEALTH_STEPS_GRAPH: return &mod_health_steps_graph_def;
        case MOD_CALENDAR_COUNTDOWN: return &mod_calendar_countdown_def;
        case MOD_CALENDAR_AGENDA: return &mod_calendar_agenda_def;
        case MOD_CALENDAR_FREEBUSY: return &mod_calendar_freebusy_def;
        case MOD_TIME_WEEKNUM: return &mod_time_weeknum_def;
        case MOD_TIME_DAYOFYEAR: return &mod_time_dayofyear_def;
        case MOD_TIME_EPOCH: return &mod_time_epoch_def;
        case MOD_TIME_YEAREND: return &mod_time_yearend_def;
        case MOD_TIME_WEEKDAY_DOTS: return &mod_time_weekday_dots_def;
        case MOD_TIME_MOON_COUNTDOWN: return &mod_time_moon_countdown_def;
        case MOD_TIME_MONTH: return &mod_time_month_def;
        case MOD_TIME_JULIAN: return &mod_time_julian_def;
        case MOD_TIME_WEEKSLEFT: return &mod_time_weeksleft_def;
        case MOD_SYSTEM_STATUS: return &mod_system_status_def;
        case MOD_TIME_HOUR_BIG: return &mod_time_hour_big_def;
        case MOD_TIME_MIN_BIG: return &mod_time_min_big_def;
        case MOD_TIME_BIGCLOCK: return &mod_time_bigclock_def;
        default: return &s_mod_empty_def;
    }
}

bool module_allows_size(ModuleType type, ModuleSize size)
{
    if (size >= MSIZE_COUNT)
    {
        return false;
    }

    return (module_def(type)->sizes & (1 << size)) != 0;
}

void modules_cleanup(void)
{
    // let each module that holds something free it. most hold nothing (their icons are
    // freed centrally below) so they leave cleanup NULL and the loop skips them
    for (uint8_t type = 0; type < MOD_TYPE_COUNT; type++)
    {
        const ModuleDef *def = module_def(type);
        if (def->cleanup)
        {
            def->cleanup();
        }
    }

    // free every cached icon picture, once, after the modules have let go of their handles
    icons_cleanup();
}
