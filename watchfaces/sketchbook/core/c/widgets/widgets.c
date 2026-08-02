/**
 * @file widgets.c
 * @brief The painted chrome: battery gauge, bluetooth glyph, and the stats-row marks.
 *
 * @ingroup family-sketchbook
 */
#include "sketchbook/widgets/widgets.h"

#include <string.h>
#include <time.h>

#include "layout.h"
#include "sketchbook/draw/fonts.h"
#include "ui/fonts.h"
#include "ui/icon_cache.h"
#include "ui/readouts.h"

/**
 * @addtogroup family-sketchbook
 * @{
 */

void widgets_draw_battery(GContext *ctx, GColor ink, uint8_t theme, int level)
{
    if (level < 0)
    {
        level = 0;
    }

    if (level > 100)
    {
        level = 100;
    }

    GRect body = BATT_RECT;

    int nub_h = body.size.h / 2;
    GRect nub = GRect(body.origin.x + body.size.w, body.origin.y + (body.size.h - nub_h) / 2, 2, nub_h);
    graphics_context_set_fill_color(ctx, ink);
    graphics_fill_rect(ctx, nub, 0, GCornerNone);

    graphics_context_set_stroke_color(ctx, ink);
    graphics_draw_rect(ctx, body);

    // five segments lit from the left by charge level
    const int segments = 5;
    const int gap = 1;
    GRect inner = GRect(body.origin.x + 2, body.origin.y + 1, body.size.w - 4, body.size.h - 2);
    int seg_w = (inner.size.w - (segments - 1) * gap) / segments;
    if (seg_w < 1)
    {
        seg_w = 1;
    }

    int lit = (level * segments + 50) / 100;  // round to nearest segment
    if (lit == 0 && level > 0)
    {
        lit = 1;  // never read empty while there's still charge
    }

    graphics_context_set_fill_color(ctx, sketchbook_battery_fill(theme, ink, level));

    for (int i = 0; i < lit; i++)
    {
        // inset 1px top and bottom so a dark margin reads as a line inside the outline
        GRect cell = GRect(inner.origin.x + i * (seg_w + gap), inner.origin.y + 1, seg_w, inner.size.h - 2);
        graphics_fill_rect(ctx, cell, 0, GCornerNone);
    }
}

void widgets_draw_top_bar(GContext *ctx, GRect bounds)
{
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, TOP_BAR_H), 0, GCornerNone);
}

void widgets_draw_date_bar(GContext *ctx, GRect bounds)
{
#if defined(PBL_ROUND)
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, TOP_BAR2_Y, bounds.size.w, TOP_BAR2_H), 0, GCornerNone);
#else
    (void)ctx;
    (void)bounds;
#endif
}

void widgets_draw_bt(GContext *ctx, bool connected)
{
    GBitmap *bmp = icon_get(connected ? RESOURCE_ID_ICON_BLUETOOTH : RESOURCE_ID_ICON_BLUETOOTH_SLASH);
    if (!bmp)
    {
        return;
    }

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, bmp, BT_ICON);
}

/**
 * @brief Draw one white master glyph recoloured to the palette.
 *
 * @param ctx The graphics context.
 * @param res The icon resource.
 * @param box Where to put it.
 * @param color The colour to paint it.
 */
static void draw_glyph(GContext *ctx, uint32_t res, GRect box, GColor color)
{
    GBitmap *bmp = icon_get(res);
    if (!bmp)
    {
        return;
    }

    icon_tint(bmp, color);
    graphics_draw_bitmap_in_rect(ctx, bmp, box);
}

/**
 * @brief How wide a stat reads on screen, and which of its two boxes it lands in.
 *
 * Mirrors the zone fit (see lib ui/zone.c): the big font wins unless it overruns the slot by
 * more than its 2px safety margin, in which case the small one does and the layer moves to the
 * nudged fallback box. The glyph has to know which way that went, or it sits 2px out of line
 * on every long reading.
 *
 * @param text The stat's text.
 * @param big The slot the big font draws in.
 * @param small The slot the small font draws in.
 * @param used Receives whichever of the two it settled on.
 * @return The rendered width in pixels.
 */
static int stat_metrics(const char *text, GRect big, GRect small, GRect *used)
{
    GRect measure = GRect(0, 0, 1000, 100);
    int width = graphics_text_layout_get_content_size(text, fonts_get(FONT_HAND_18), measure,
        GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft).w;

    if (width <= big.size.w - 2)
    {
        *used = big;
        return width;
    }

    *used = small;
    return graphics_text_layout_get_content_size(text, fonts_get(FONT_HAND_16), measure,
        GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft).w;
}

/** @brief The width of a glyph's visible art, with its transparent padding backed out. */
static int glyph_art_width(uint32_t res)
{
    IconMargins margins = icon_margins(res);
    return icon_size(res).w - margins.e - margins.w;
}

/**
 * @brief Sit a glyph's visible art on the stats-row baseline, starting at @p art_left.
 *
 * Each bundled glyph keeps a little transparent padding and how much differs per icon, so
 * pinning the bitmap box would leave the row sitting at three different heights. The measured
 * margins are backed out and the art itself is what gets placed.
 *
 * @param res The icon resource.
 * @param art_left Where the visible art should start.
 * @return The box to draw the bitmap in.
 */
static GRect glyph_on_line(uint32_t res, int art_left, int baseline)
{
    GSize size = icon_size(res);
    IconMargins margins = icon_margins(res);
    int art_h = size.h - margins.n - margins.s;

    return GRect(art_left - margins.w, baseline - art_h - margins.n, size.w, size.h);
}

/** @brief The row's own baseline, for a mark that sits beside its number. */
static GRect glyph_on_baseline(uint32_t res, int art_left)
{
    return glyph_on_line(res, art_left, STAT_BASELINE);
}

/** @brief Sit a glyph's art on the baseline, ending a gap short of where its number starts. */
static GRect glyph_before(uint32_t res, int text_left)
{
    return glyph_on_baseline(res, text_left - STAT_GLYPH_GAP - glyph_art_width(res));
}

/** @brief The same, on a row that sits somewhere other than the face's default line. */
static GRect glyph_before_on(uint32_t res, int text_left, int baseline)
{
    return glyph_on_line(res, text_left - STAT_GLYPH_GAP - glyph_art_width(res), baseline);
}

bool sketchbook_has_hr(void)
{
#if defined(PBL_PLATFORM_GABBRO)
    // this watch has no sensor to ask. the health service still reports the metric as accessible
    // on the emulator, so taking its word for it would leave a slot that can only say "--"
    return false;
#elif defined(PBL_HEALTH)
    time_t now = time(NULL);
    HealthServiceAccessibilityMask access =
        health_service_metric_accessible(HealthMetricHeartRateBPM, now, now);
    return access & HealthServiceAccessibilityMaskAvailable;
#else
    return false;
#endif
}

void widgets_draw_stat_glyphs(GContext *ctx, const Palette *pal, const Zone *weather, const Zone *steps)
{
    char text[16];

    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    GRect slot;
    bool has_hr = sketchbook_has_hr();

#if defined(PBL_ROUND)
    // the round faces carry a single unlabelled reading, so there is nothing to mark
    if (!has_hr)
    {
        (void)slot;
        (void)text;
        return;
    }
#endif

    // every mark reads its number's box back off the zone, so it follows both a font fallback and
    // a layout that moved the whole row somewhere else
    readout_weather_temp(text, sizeof(text));
    if (has_hr)
    {
        // three across, and the thermometer is the fixed one: its number is left-aligned off it,
        // so it has nothing to follow
        draw_glyph(ctx, RESOURCE_ID_ICON_THERMOMETER,
            glyph_on_baseline(RESOURCE_ID_ICON_THERMOMETER, STAT_TEMP_GLYPH_X), pal->dim);
    }
    else
    {
        int temp_w = stat_metrics(text, weather->rect, weather->rect_fallback, &slot);
        int temp_mid = slot.origin.x + slot.size.w / 2;
        int temp_base = STAT_ROW_BASELINE(weather->rect);
        draw_glyph(ctx, RESOURCE_ID_ICON_THERMOMETER,
            glyph_before_on(RESOURCE_ID_ICON_THERMOMETER, temp_mid - temp_w / 2, temp_base), pal->dim);
    }

    if (has_hr)
    {
        readout_hr(text, sizeof(text));
        int hr_w = stat_metrics(text, SLOT_HR, SLOT_HR_SM, &slot);
        int hr_left = slot.origin.x + slot.size.w / 2 - hr_w / 2;  // centred
        draw_glyph(ctx, RESOURCE_ID_ICON_HEART, glyph_before(RESOURCE_ID_ICON_HEART, hr_left), pal->dim);
    }

    readout_steps(text, sizeof(text));
    if (has_hr)
    {
        int steps_w = stat_metrics(text, SLOT_STEPS, SLOT_STEPS_SM, &slot);
        int steps_left = slot.origin.x + slot.size.w - steps_w;  // right-aligned
        draw_glyph(ctx, RESOURCE_ID_ICON_FEET, glyph_before(RESOURCE_ID_ICON_FEET, steps_left), pal->dim);
    }
    else
    {
        int steps_w = stat_metrics(text, steps->rect, steps->rect_fallback, &slot);
        int steps_mid = slot.origin.x + slot.size.w / 2;
        int steps_base = STAT_ROW_BASELINE(steps->rect);
        draw_glyph(ctx, RESOURCE_ID_ICON_FEET,
            glyph_before_on(RESOURCE_ID_ICON_FEET, steps_mid - steps_w / 2, steps_base), pal->dim);
    }
}

/** @brief How wide @p text renders in @p font. */
static int text_width(const char *text, GFont font)
{
    return graphics_text_layout_get_content_size(text, font, GRect(0, 0, 1000, 200),
        GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft).w;
}

/**
 * @brief The marker text and the clock it is placed against, or false when there is no marker.
 *
 * @param marker Receives the AM/PM text.
 * @param marker_size Its buffer size.
 * @param clock Receives the clock text.
 * @param clock_size Its buffer size.
 * @return False on a 24-hour or .beats clock, which is the only time nothing is drawn.
 */
static bool meridiem_strings(char *marker, size_t marker_size, char *clock, size_t clock_size)
{
    readout_meridiem(marker, marker_size);
    if (!marker[0])
    {
        return false;
    }

    readout_time(clock, clock_size);
    return true;
}

void widgets_draw_meridiem_beside(GContext *ctx, GColor color, GRect clock_slot, FontId clock_font, int top)
{
    char marker[4];
    char clock[12];
    if (!meridiem_strings(marker, sizeof(marker), clock, sizeof(clock)))
    {
        return;
    }

    // the clock is centred, so where its edges land moves with the format. measuring it is what
    // keeps the marker tucked against the digits instead of floating off to one side
    int clock_w = text_width(clock, fonts_get(clock_font));
    int centre = clock_slot.origin.x + clock_slot.size.w / 2;

    graphics_context_set_text_color(ctx, color);

    // the box is pinned to the digits' edge and the text pulled to the near end, which keeps the
    // gap even on both sides
    if (marker[0] == 'P')
    {
        graphics_draw_text(ctx, marker, fonts_get(FONT_HAND_16),
            GRect(centre + clock_w / 2 + MERIDIEM_GAP, top, MERIDIEM_BESIDE_W, 20),
            GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }
    else
    {
        graphics_draw_text(ctx, marker, fonts_get(FONT_HAND_16),
            GRect(centre - clock_w / 2 - MERIDIEM_GAP - MERIDIEM_BESIDE_W, top, MERIDIEM_BESIDE_W, 20),
            GTextOverflowModeFill, GTextAlignmentRight, NULL);
    }
}

void widgets_draw_meridiem_above(GContext *ctx, GColor color, GRect clock_slot, FontId clock_font, int top)
{
    char marker[4];
    char clock[12];
    if (!meridiem_strings(marker, sizeof(marker), clock, sizeof(clock)))
    {
        return;
    }

    const char *colon = strchr(clock, ':');
    if (!colon)
    {
        return;  // no colon means no channel to sit in
    }

    // the hours stand between the clock's left edge and the colon, so measuring them finds the
    // gap. it moves with the time: "08" is wider than "10"
    char hours[8];
    size_t len = (size_t)(colon - clock);
    if (len >= sizeof(hours))
    {
        return;
    }
    memcpy(hours, clock, len);
    hours[len] = '\0';

    GFont font = fonts_get(clock_font);
    int left = clock_slot.origin.x + clock_slot.size.w / 2 - text_width(clock, font) / 2;
    int colon_cx = left + text_width(hours, font) + text_width(":", font) / 2;

    graphics_context_set_text_color(ctx, color);
    graphics_draw_text(ctx, marker, fonts_get(FONT_HAND_16),
        GRect(colon_cx - MERIDIEM_W / 2, top, MERIDIEM_W, 20),
        GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

void widgets_draw_qt(GContext *ctx, GColor color)
{
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    draw_glyph(ctx, RESOURCE_ID_ICON_VOLUME_MUTED, QT_ICON, color);
}

void widgets_draw_qt_state(GContext *ctx, GColor color, bool active)
{
#if defined(HAS_QUIET_PAIR)
    // a matched pair, the way bluetooth has one: the slot is filled either way so the strip keeps
    // its shape instead of going lopsided whenever Quiet Time is off
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    draw_glyph(ctx, active ? RESOURCE_ID_ICON_QUIET_ON : RESOURCE_ID_ICON_QUIET_OFF, QT_ICON, color);
#else
    if (active)
    {
        widgets_draw_qt(ctx, color);
    }
#endif
}

/** @} */
