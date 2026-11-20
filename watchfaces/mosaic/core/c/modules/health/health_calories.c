#include "health_calories.h"
#include "mosaic/draw/stat_panel.h"
#include "io/stores/health_store.h"
#include "health/goals.h"
#include "system/settings/settings.h"
#include "text/number_format.h"
#include <stdio.h>

static void cal_value_1x2(char *out, size_t n, const char **unit_out)
{
    if (health_store_calories() < 0)
    {
        snprintf(out, n, "--");
    }
    else
    {
        number_group(out, n, health_store_calories());
    }
}

static void cal_goal(char *out, size_t n, int *value, int *goal)
{
    int target = goals_calories(settings_u8(SETTING_GOAL_CALORIES));
    number_group(out, n, target);
    *value = health_store_calories();
    *goal = target;
}

static const StatPanelDesc calories_desc = {
    .base = {
        .value_1x2 = cal_value_1x2,
        .icon = &ICON_FIRE,
        .font = FONT_TEKO_26,
    },
    .goal_2x2 = cal_goal,
    .goal_caption = "GOAL",
};

STAT_PANEL_GOAL(health_calories, "CALORIES", SZ_1x2 | SZ_2x2, FEATURE_HEALTH, 0, &calories_desc);
STAT_PANEL_GOAL_BAR(health_calories_goal, "CALORIE GOAL", FEATURE_HEALTH, MOD_HEALTH_CALORIES, &calories_desc);
