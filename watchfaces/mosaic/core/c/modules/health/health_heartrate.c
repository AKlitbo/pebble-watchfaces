#include "health_heartrate.h"
#include "mosaic/draw/stat_panel.h"
#include "text/number_format.h"
#include "mosaic/draw/common.h"
#include "io/stores/health_store.h"
#include "settings_schema.h"
#include <stdio.h>

static void hr_value_1x2(char *out, size_t n, const char **unit_out)
{
    fmt_int_or_dash(out, n, health_store_hr(), "%d");
    *unit_out = "BPM";
}

static void hr_goal(char *out, size_t n, int *value, int *goal)
{
    snprintf(out, n, "%d BPM", gridlock_hr_limit());
    *value = health_store_hr();
    *goal = gridlock_hr_limit();
}

static const StatPanelDesc heartrate_desc = {
    .base = {
        .value_1x2 = hr_value_1x2,
        .icon = &ICON_HEART,
        .font = FONT_TEKO_26,
    },
    .goal_2x2 = hr_goal,
    .goal_caption = "LIMIT",
};

STAT_PANEL_GOAL(health_heartrate, "HEARTRATE", SZ_1x2 | SZ_2x2, FEATURE_HEALTH, 0, &heartrate_desc);
