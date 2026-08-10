#include "engine/grid_engine.h"
#include "time_month.h"
#include "io/stores/time_store.h"
#include "settings_schema.h"
#include "theme/theme.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "clock/date.h"
#include "clock/weekday.h"
#include "text/text_case.h"
#include <time.h>
#include <stdio.h>

// the letters row height. the day rows split whatever is left
#define MONTH_HEAD_H 11
// the text boxes are one Share Tech Mono line tall
#define MONTH_LINE_H 12

static void time_month_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x4)
    {
        return;
    }

    const struct tm *t = time_store_tm();
    int first = gridlock_week_start();
    int days = date_days_in_month(t->tm_year + 1900, t->tm_mon);
    // blank cells before the 1st once the columns start on the configured day
    int lead = (date_first_wday(t->tm_wday, t->tm_mday) - first + 7) % 7;
    int rows = (lead + days + 6) / 7;

    GFont font = fonts_get(FONT_STM_12);

    // seven equal columns centred in the body
    int inner_w = gctx->body.size.w - PANEL_PAD * 2;
    int col_w = inner_w / 7;
    int x0 = gctx->body.origin.x + PANEL_PAD + (inner_w - col_w * 7) / 2;
    int y0 = gctx->body.origin.y;
    int pitch = (gctx->body.size.h - MONTH_HEAD_H) / rows;

    // single letter weekday strip in the caption colour
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    for (int col = 0; col < 7; col++)
    {
        char letter[2] = { weekday_short((col + first) % 7)[0], '\0' };
        GRect box = GRect(x0 + col * col_w, y0 - 2, col_w, MONTH_LINE_H);
        graphics_draw_text(gctx->ctx, letter, font, box,
                           GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }

    for (int day = 1; day <= days; day++)
    {
        int cell = lead + day - 1;
        // the -1 lifts the day rows off the bottom edge so the last week does not ride the border
        GRect box = GRect(x0 + (cell % 7) * col_w, y0 + MONTH_HEAD_H - 1 + (cell / 7) * pitch,
                          col_w, MONTH_LINE_H);

        if (day == t->tm_mday)
        {
            // fill behind today and flip its number to the background colour. the +3 seats the
            // box on the number rather than riding above it
            graphics_context_set_fill_color(gctx->ctx, gctx->color_value);
            graphics_fill_rect(gctx->ctx, GRect(box.origin.x, box.origin.y + 3, col_w, pitch),
                               0, GCornerNone);
            graphics_context_set_text_color(gctx->ctx,
                                            theme_background(settings_u8(SETTING_THEME)));
        }
        else
        {
            graphics_context_set_text_color(gctx->ctx, gctx->color_value);
        }

        char num[12];
        snprintf(num, sizeof(num), "%d", day);
        graphics_draw_text(gctx->ctx, num, font, box,
                           GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
}

static const char *time_month_label(ModuleSize size)
{
    // the header names the month being shown rather than the panel
    static char label[12];
    strftime(label, sizeof(label), "%B", time_store_tm());
    text_to_upper(label);
    return label;
}

const ModuleDef mod_time_month_def = {
    .label = "MONTH",
    .get_label = time_month_label,
    .sizes = SZ_2x4,
    .features = FEATURE_TIME,
    .body = time_month_body,
};
