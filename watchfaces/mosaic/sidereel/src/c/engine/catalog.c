/**
 * @file catalog.c
 * @brief The module catalog, limited to the panels that fit this face's two cell sizes.
 *
 * sidereel has a 1x2 and a 2x2, so a module gridlock allows at either is carried here. The wider
 * ones are not: a 1x4 or 2x4 needs the full screen width, and this face gives its right half to
 * the reel.
 *
 * The ModuleType enum keeps every id gridlock has, including the ones with no case below, so a
 * layout string and a Clay option value mean the same thing on both faces. An id with no case
 * lands on the empty module and draws nothing.
 *
 * @ingroup watchface-sidereel
 */
#include "mosaic/engine/catalog.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/icons.h"

#include "mosaic/modules/health/health_activity.h"
#include "mosaic/modules/health/health_calories.h"
#include "mosaic/modules/health/health_distance.h"
#include "mosaic/modules/health/health_heartrate.h"
#include "mosaic/modules/health/health_hr_graph.h"
#include "mosaic/modules/health/health_sleep.h"
#include "mosaic/modules/health/health_steps.h"
#include "mosaic/modules/health/health_steps_graph.h"
#include "mosaic/modules/system/system_battery.h"
#include "mosaic/modules/time/time_beats.h"
#include "mosaic/modules/time/time_date.h"
#include "mosaic/modules/time/time_dayofyear.h"
#include "mosaic/modules/time/time_epoch.h"
#include "mosaic/modules/time/time_julian.h"
#include "mosaic/modules/time/time_moon.h"
#include "mosaic/modules/time/time_moon_countdown.h"
#include "mosaic/modules/time/time_weekday_dots.h"
#include "mosaic/modules/time/time_weeknum.h"
#include "mosaic/modules/time/time_weeksleft.h"
#include "mosaic/modules/time/time_yearend.h"
#include "mosaic/modules/time/time_zone.h"
#include "mosaic/modules/weather/weather_conditions.h"
#include "mosaic/modules/weather/weather_current.h"
#include "mosaic/modules/weather/weather_daylight.h"
#include "mosaic/modules/weather/weather_dew_point.h"
#include "mosaic/modules/weather/weather_feels_like.h"
#include "mosaic/modules/weather/weather_hilo.h"
#include "mosaic/modules/weather/weather_humidity.h"
#include "mosaic/modules/weather/weather_precip.h"
#include "mosaic/modules/weather/weather_pressure.h"
#include "mosaic/modules/weather/weather_solar.h"
#include "mosaic/modules/weather/weather_sun.h"
#include "mosaic/modules/weather/weather_temperature.h"
#include "mosaic/modules/weather/weather_uv.h"
#include "mosaic/modules/weather/weather_wind.h"

#include <string.h>

#include "ui/fonts.h"
#include "system/units/units.h"
#include "ui/weather/icons.h"
#include "text/number_format.h"
#include "text/text_case.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "settings_schema.h"

static const ModuleDef s_mod_empty_def = {.label = "EMPTY", .sizes = 0};

const ModuleDef *module_def(ModuleType type)
{
    switch (type)
    {
        case MOD_SYSTEM_BATTERY: return &mod_system_battery_def;

        case MOD_HEALTH_HEARTRATE: return &mod_health_heartrate_def;
        case MOD_HEALTH_STEPS: return &mod_health_steps_def;
        case MOD_HEALTH_DISTANCE: return &mod_health_distance_def;
        case MOD_HEALTH_CALORIES: return &mod_health_calories_def;
        case MOD_HEALTH_SLEEP: return &mod_health_sleep_def;
        case MOD_HEALTH_ACTIVITY: return &mod_health_activity_def;
        case MOD_HEALTH_HR_GRAPH: return &mod_health_hr_graph_def;
        case MOD_HEALTH_STEPS_GRAPH: return &mod_health_steps_graph_def;

        case MOD_TIME_BEATS: return &mod_time_beats_def;
        case MOD_TIME_DATE: return &mod_time_date_def;
        case MOD_TIME_DAYOFYEAR: return &mod_time_dayofyear_def;
        case MOD_TIME_EPOCH: return &mod_time_epoch_def;
        case MOD_TIME_JULIAN: return &mod_time_julian_def;
        case MOD_TIME_MOON: return &mod_time_moon_def;
        case MOD_TIME_MOON_COUNTDOWN: return &mod_time_moon_countdown_def;
        case MOD_TIME_WEEKDAY_DOTS: return &mod_time_weekday_dots_def;
        case MOD_TIME_WEEKNUM: return &mod_time_weeknum_def;
        case MOD_TIME_WEEKSLEFT: return &mod_time_weeksleft_def;
        case MOD_TIME_YEAREND: return &mod_time_yearend_def;
        case MOD_TIME_ZONE_1: return &mod_time_zone_1_def;

        case MOD_WEATHER_CURRENT: return &mod_weather_current_def;
        case MOD_WEATHER_CONDITIONS: return &mod_weather_conditions_def;
        case MOD_WEATHER_DAYLIGHT: return &mod_weather_daylight_def;
        case MOD_WEATHER_TEMPERATURE: return &mod_weather_temperature_def;
        case MOD_WEATHER_FEELS_LIKE: return &mod_weather_feels_like_def;
        case MOD_WEATHER_HUMIDITY: return &mod_weather_humidity_def;
        case MOD_WEATHER_WIND: return &mod_weather_wind_def;
        case MOD_WEATHER_UV: return &mod_weather_uv_def;
        case MOD_WEATHER_HILO: return &mod_weather_hilo_def;
        case MOD_WEATHER_PRECIP: return &mod_weather_precip_def;
        case MOD_WEATHER_PRESSURE: return &mod_weather_pressure_def;
        case MOD_WEATHER_DEW_POINT: return &mod_weather_dew_point_def;
        case MOD_WEATHER_SUNRISE: return &mod_weather_sunrise_def;
        case MOD_WEATHER_SUNSET: return &mod_weather_sunset_def;
        case MOD_WEATHER_SOLAR: return &mod_weather_solar_def;

        // everything else is a gridlock module that needs more than a 1x2 cell, or a store this
        // face does not wire. the id stays valid and simply draws nothing
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
