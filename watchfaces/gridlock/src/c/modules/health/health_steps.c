#include "health_steps.h"
#include "draw/stat_panel.h"
#include "io/stores/health_store.h"
#include "settings_schema.h"
#include "system/settings/setting_values.h"
#include "system/units/units.h"
#include "text/number_format.h"
#include <stdio.h>

// fills `val` with the steps readout honouring the STEPS_MODE setting: a grouped step
// count by default, or today's walked distance in miles/km when the user picks one.
// returns the trailing unit label ("MI"/"KM") for distance mode, or NULL when the
// value stands alone (a bare step count, or the no-data placeholder).
static const char *format_steps_readout(char *val, size_t n)
{
    if (health_store_steps() < 0)
    {
        // no step reading yet means no distance either, so one placeholder covers both
        snprintf(val, n, "--");
        return NULL;
    }

    uint8_t mode = settings_u8(SETTING_STEPS_MODE);
    if (mode == STEPS_MODE_MILES || mode == STEPS_MODE_KM)
    {
        bool miles = (mode == STEPS_MODE_MILES);
        units_format_distance_value(val, n, health_store_distance_m(), miles);
        return units_distance_unit(miles);
    }

    number_group(val, n, health_store_steps());
    return NULL;
}

static void steps_value_1x2(char *out, size_t n, const char **unit_out)
{
    // the MI/KM label (if any) drops to the small trailing font, matching BPM and MIN
    *unit_out = format_steps_readout(out, n);
}

static void steps_value_2x2(char *out, size_t n)
{
    // no small-font slot here, so fold the unit back onto the big value
    char val[16];
    const char *unit = format_steps_readout(val, sizeof(val));
    if (unit)
    {
        snprintf(out, n, "%s %s", val, unit);
    }
    else
    {
        snprintf(out, n, "%s", val);
    }
}

// the goal is always a step count, whatever the readout above it says. STEPS_MODE can headline
// today's distance instead, so the caption names the unit and the two lines stop looking like a
// distance measured against a distance
static void steps_goal(char *out, size_t n, int *value, int *goal)
{
    number_group(out, n, gridlock_goal_steps());
    *value = health_store_steps();
    *goal = gridlock_goal_steps();
}

static const StatPanelDesc steps_desc = {
    .base = {
        .value_1x2 = steps_value_1x2,
        .icon = &ICON_FEET,
        .font = FONT_TEKO_26,
    },
    .value_2x2 = steps_value_2x2,
    .goal_2x2 = steps_goal,
    .goal_caption = "STEP GOAL",
};

STAT_PANEL_GOAL(health_steps, "STEPS", SZ_1x2 | SZ_2x2, FEATURE_HEALTH, 0, &steps_desc);
