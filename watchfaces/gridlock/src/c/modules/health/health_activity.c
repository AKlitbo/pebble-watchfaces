#include "health_activity.h"
#include "draw/stat_panel.h"
#include "text/number_format.h"
#include "draw/common.h"
#include "io/stores/health_store.h"
#include "settings_schema.h"
#include <stdio.h>

static void act_value_1x2(char *out, size_t n, const char **unit_out)
{
    fmt_int_or_dash(out, n, health_store_active_min(), "%d");
    *unit_out = "MIN";
}

// the goal path prepends "OF ", so "%d MIN" reads as "OF <goal> MIN" like it did by hand
static void act_goal(char *out, size_t n, int *value, int *goal)
{
    snprintf(out, n, "%d MIN", gridlock_goal_active_min());
    *value = health_store_active_min();
    *goal = gridlock_goal_active_min();
}

static const StatPanelDesc activity_desc = {
    .base = {
        .value_1x2 = act_value_1x2,
        .icon = &ICON_TIME_LATE,
        .font = FONT_TEKO_26,
    },
    .goal_2x2 = act_goal,
    .goal_caption = "GOAL",
};

STAT_PANEL_GOAL(health_activity, "ACTIVITY", SZ_1x2 | SZ_2x2, FEATURE_HEALTH, 0, &activity_desc);
