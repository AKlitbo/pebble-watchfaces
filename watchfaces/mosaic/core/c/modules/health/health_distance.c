#include "health_distance.h"
#include "mosaic/draw/stat_panel.h"
#include "io/stores/health_store.h"
#include "settings_schema.h"
#include "system/units/units.h"
#include <stdio.h>

// the health service hands steps and distance in together, so no step reading yet means no
// distance either
static bool distance_no_data(void)
{
    return health_store_steps() < 0;
}

static void dist_value_1x2(char *out, size_t n, const char **unit_out)
{
    if (distance_no_data())
    {
        snprintf(out, n, "--");
        return;
    }

    // the number rides the big value font, the MI/KM label its own small slot
    bool miles = gridlock_distance_in_miles();
    units_format_distance_value(out, n, health_store_distance_m(), miles);
    *unit_out = units_distance_unit(miles);
}

static void dist_value_2x2(char *out, size_t n)
{
    if (distance_no_data())
    {
        snprintf(out, n, "--");
        return;
    }

    units_format_distance(out, n, health_store_distance_m(), gridlock_distance_in_miles());
}

static void dist_goal(char *out, size_t n, int *value, int *goal)
{
    units_format_distance(out, n, gridlock_goal_distance_m(), gridlock_distance_in_miles());
    *value = health_store_distance_m();
    *goal = gridlock_goal_distance_m();
}

static const StatPanelDesc distance_desc = {
    .base = {
        .value_1x2 = dist_value_1x2,
        .icon = &ICON_DISTANCE,
        .font = FONT_TEKO_26,
    },
    .value_2x2 = dist_value_2x2,
    .goal_2x2 = dist_goal,
    .goal_caption = "GOAL",
};

STAT_PANEL_GOAL(health_distance, "DISTANCE", SZ_1x2 | SZ_2x2, FEATURE_HEALTH, 0, &distance_desc);
