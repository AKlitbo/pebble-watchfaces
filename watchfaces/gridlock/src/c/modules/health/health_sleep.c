#include "health_sleep.h"
#include "clock/duration.h"
#include "draw/stat_panel.h"
#include "io/stores/health_store.h"
#include "settings_schema.h"
#include <stdio.h>

static void sleep_value_1x2(char *out, size_t n, const char **unit_out)
{
    int min = health_store_sleep_min();
    if (min < 0)
    {
        snprintf(out, n, "--");
    }
    else
    {
        duration_hm(out, n, min);
    }
    *unit_out = "HRS";
}

static void sleep_value_2x2(char *out, size_t n)
{
    int min = health_store_sleep_min();
    if (min < 0)
    {
        snprintf(out, n, "--");
    }
    else
    {
        snprintf(out, n, "%dH %dM", min / 60, min % 60);
    }
}

static void sleep_goal(char *out, size_t n, int *value, int *goal)
{
    int g = gridlock_goal_sleep_min();
    if (g % 60 == 0)
    {
        snprintf(out, n, "%dH", g / 60);
    }
    else
    {
        snprintf(out, n, "%dH %dM", g / 60, g % 60);
    }
    *value = health_store_sleep_min();
    *goal = g;
}

static const StatPanelDesc sleep_desc = {
    .base = {
        .value_1x2 = sleep_value_1x2,
        .icon = &ICON_SNOOZE,
        .font = FONT_TEKO_26,
    },
    .value_2x2 = sleep_value_2x2,
    .goal_2x2 = sleep_goal,
    .goal_caption = "GOAL",
};

STAT_PANEL_GOAL(health_sleep, "SLEEP", SZ_1x2 | SZ_2x2, FEATURE_HEALTH, 0, &sleep_desc);
