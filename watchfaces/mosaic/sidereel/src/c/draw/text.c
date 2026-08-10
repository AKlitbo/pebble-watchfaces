/**
 * @file text.c
 * @brief How this face writes a clock.
 *
 * @ingroup watchface-sidereel
 */
#include "text.h"

#include "mosaic/draw/metrics.h"
#include "system/settings/setting_values.h"
#include "system/settings/settings.h"
#include "ui/fonts.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

void side_draw_centred(GContext *ctx, const char *text, FontId font, GRect box)
{
    const FontMetric *metric = metric_for(font);

    // a slot with no metrics record would otherwise collapse the rect to nothing and draw
    // silently, so fall back to the box itself and accept the line sitting a little high
    int height = metric->line_h ? metric->line_h + metric->top_pad : box.size.h;

    // sit the caps in the middle of the box, then lift by the see-through rows the font keeps
    // above them
    int top = box.origin.y + (box.size.h - metric->cap_h) / 2 - metric->top_pad;

    graphics_draw_text(ctx, text, fonts_get(font),
                       GRect(box.origin.x, top, box.size.w, height),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

bool side_is_12h(void)
{
    switch (settings_u8(SETTING_TIME_FORMAT))
    {
        case TIME_FORMAT_12H:
        case TIME_FORMAT_12H_NO_LEAD:
            return true;

        case TIME_FORMAT_24H:
        case TIME_FORMAT_BEATS:  // no hour of its own to write, so read it as 24 hour
            return false;

        default:
            return !clock_is_24h_style();
    }
}

// 0 and 12 both read as 12 on a 12 hour clock
static int clock_hour(int hour)
{
    if (!side_is_12h())
    {
        return hour;
    }

    int shown = hour % 12;

    return shown == 0 ? 12 : shown;
}

void side_format_clock(char *out, size_t size, int hour, int minute)
{
    bool lead = settings_u8(SETTING_TIME_FORMAT) != TIME_FORMAT_12H_NO_LEAD;

    snprintf(out, size, lead ? "%02d:%02d" : "%d:%02d", clock_hour(hour), minute);
}

void side_format_hour(char *out, size_t size, int hour)
{
    bool lead = settings_u8(SETTING_TIME_FORMAT) != TIME_FORMAT_12H_NO_LEAD;

    snprintf(out, size, lead ? "%02d" : "%d", clock_hour(hour));
}

/** @} */
