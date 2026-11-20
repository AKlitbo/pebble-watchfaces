#include "health_activity.h"
#include "mosaic/draw/stat_panel.h"
#include "text/number_format.h"
#include "mosaic/draw/common.h"
#include "io/stores/health_store.h"
#include "health/goals.h"
#include "system/settings/settings.h"
#include <stdio.h>

static void act_value_1x2(char *out, size_t n, const char **unit_out)
{
    fmt_int_or_dash(out, n, health_store_active_min(), "%d");
    *unit_out = "MIN";
}

// the goal path puts "OF " in front, so "%d MIN" reads as "OF <goal> MIN"
static void act_goal(char *out, size_t n, int *value, int *goal)
{
    int target = goals_active_min(settings_u8(SETTING_GOAL_ACTIVE));
    snprintf(out, n, "%d MIN", target);
    *value = health_store_active_min();
    *goal = target;
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
STAT_PANEL_GOAL_BAR(health_activity_goal, "ACTIVE GOAL", FEATURE_HEALTH, MOD_HEALTH_ACTIVITY, &activity_desc);
