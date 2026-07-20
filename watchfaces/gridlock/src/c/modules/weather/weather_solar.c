/**
 * @file weather_solar.c
 * @brief The "Sun" module. Reads the phone's sunrise and sunset times plus the current
 * clock and shows how the day's light is going. It draws three ways:
 *   1x2  a countdown to the next sun event (sunrise while it is dark, sunset while it is up)
 *   2x2  a sun riding an arc at the current day-progress, with the rise and set times
 *   1x4  a full-width sunrise to sunset track with a marker at now and the three times
 *
 * All three read the same rise/set strings from the weather store and the same clock from
 * the time store, so nothing here talks to a service directly.
 * @ingroup gridlock_mod_weather
 */
#include "engine/grid_engine.h"
#include "weather_solar.h"
#include "clock/clockstr.h"
#include "clock/duration.h"
#include "clock/solar.h"
#include "draw/grid_helpers.h"
#include "draw/icons.h"
#include "draw/metrics.h"
#include "io/stores/weather_store.h"
#include "io/stores/time_store.h"
#include "theme/theme.h"
#include "settings_schema.h"
#include <stdio.h>
#include <string.h>

// the rise/set times and the clock, each minutes past midnight or -1 for no reading
typedef struct
{
    int rise;
    int set;
    int now;
} SolarTimes;

static SolarTimes read_solar(void)
{
    SolarTimes s;
    s.rise = clockstr_minutes(weather_store_sunrise());
    s.set = clockstr_minutes(weather_store_sunset());

    const struct tm *t = time_store_tm();
    s.now = t ? t->tm_hour * 60 + t->tm_min : -1;

    return s;
}

// a clock time from minutes past midnight, honouring the 12/24h setting, or "--"
static void fmt_clock(char *out, size_t n, int minutes)
{
    if (minutes < 0)
    {
        snprintf(out, n, "--");
        return;
    }
    gridlock_format_clock(out, n, minutes / 60, minutes % 60);
}

// draws text into a box with the font's top padding taken off so it lands where the box says
static void draw_text(GridCtx *gctx, const char *text, FontId font, GColor color, GRect box,
                      GTextAlignment align)
{
    box = metric_baseline(font, box);
    graphics_context_set_text_color(gctx->ctx, color);
    graphics_draw_text(gctx->ctx, text, fonts_get(font), box, GTextOverflowModeFill, align, NULL);
}

// --- 1x2: the countdown ---

static void draw_countdown_tile(GridCtx *gctx)
{
    SolarTimes s = read_solar();

    bool is_sunrise = false;
    char buf[16];
    int delta = solar_next_event(s.rise, s.set, s.now, &is_sunrise);
    if (delta >= 0)
    {
        duration_hm_compact(buf, sizeof(buf), delta);
    }
    else
    {
        snprintf(buf, sizeof(buf), "--");
        is_sunrise = true;
    }

    // the icon says which event we are counting to. gh_stat_1x2 grows the value on a taller
    // headerless tile on its own
    gh_stat_1x2(gctx, buf, NULL, FONT_TEKO_26, is_sunrise ? &ICON_SUNRISE : &ICON_SUNSET);
}

// --- 2x2: the sun on an arc ---

// eight little rays around the sun so it reads as a sun and not just a dot
static void draw_sun(GridCtx *gctx, GPoint c, int disc_r, int ray_r)
{
    graphics_context_set_fill_color(gctx->ctx, gctx->color_icon);
    graphics_fill_circle(gctx->ctx, c, disc_r);

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_icon);
    for (int deg = 0; deg < 360; deg += 45)
    {
        int32_t a = DEG_TO_TRIGANGLE(deg);
        int inner = disc_r + 2;
        GPoint p0 = GPoint(c.x + (sin_lookup(a) * inner) / TRIG_MAX_RATIO,
                           c.y - (cos_lookup(a) * inner) / TRIG_MAX_RATIO);
        GPoint p1 = GPoint(c.x + (sin_lookup(a) * ray_r) / TRIG_MAX_RATIO,
                           c.y - (cos_lookup(a) * ray_r) / TRIG_MAX_RATIO);
        graphics_draw_line(gctx->ctx, p0, p1);
    }
}

// a crescent: a full disc in the icon colour with a second disc in the background colour
// punched out of it, nudged up and to the right so the bite lands on the top-right
static void draw_moon(GridCtx *gctx, GPoint c, int r)
{
    graphics_context_set_fill_color(gctx->ctx, gctx->color_icon);
    graphics_fill_circle(gctx->ctx, c, r);

    graphics_context_set_fill_color(gctx->ctx, theme_background(settings_u8(SETTING_THEME)));
    graphics_fill_circle(gctx->ctx, GPoint(c.x + r / 2, c.y - r / 2), r);
}

static void draw_arc_panel(GridCtx *gctx)
{
    GRect body = gctx->body;
    int width = body.size.w;
    int height = body.size.h;
    SolarTimes s = read_solar();

    // the dome sits above a flat baseline, leaving a strip at the bottom for the rise/set
    // times. its radius comes from the width so it spans most of the panel, pulled in a
    // couple of px each side so the feet do not crowd the corner times
    int baseline_y = body.origin.y + height - 16;
    int radius = (width - (PANEL_PAD + 4) * 2) / 2;
    GPoint centre = GPoint(body.origin.x + width / 2, baseline_y);
    GRect arc_rect = GRect(centre.x - radius, centre.y - radius, radius * 2, radius * 2);

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
    graphics_context_set_stroke_width(gctx->ctx, 2);
    // the top half of the circle: from the left end up over the top to the right end
    graphics_draw_arc(gctx->ctx, arc_rect, GOvalScaleModeFitCircle,
                      DEG_TO_TRIGANGLE(-90), DEG_TO_TRIGANGLE(90));
    graphics_context_set_stroke_width(gctx->ctx, 1);

    // the sun rides the dome at the day's progress, and the moon rides it the same way
    // through the night. only ever one of them is up at a time
    int prog = solar_day_progress(s.rise, s.set, s.now);
    int nprog = solar_night_progress(s.rise, s.set, s.now);
    if (prog >= 0)
    {
        int32_t ang = DEG_TO_TRIGANGLE(-90 + (prog * 180) / 100);
        GPoint sun = gpoint_from_polar(arc_rect, GOvalScaleModeFitCircle, ang);
        draw_sun(gctx, sun, 4, 8);
    }
    else if (nprog >= 0)
    {
        int32_t ang = DEG_TO_TRIGANGLE(-90 + (nprog * 180) / 100);
        GPoint moon = gpoint_from_polar(arc_rect, GOvalScaleModeFitCircle, ang);
        draw_moon(gctx, moon, 5);
    }

    // the countdown fills the hollow under the dome. the sunset time in the corner and the
    // sun's spot on the arc already say which way it is going, so no "til set" caption here
    bool is_sunrise = false;
    char count[16];
    int delta = solar_next_event(s.rise, s.set, s.now, &is_sunrise);
    if (delta >= 0)
    {
        duration_hm_compact(count, sizeof(count), delta);
    }
    else
    {
        snprintf(count, sizeof(count), "--");
    }

    // a two-digit-hour value like "12h 30m" is too wide for Teko 24 and clips, so drop those
    // to the narrow cut. shorter values ("2h 30m", "45m") keep Teko 24. measuring the width
    // is no help here: the layout size under-reports Teko's real ink
    FontId count_font = (strlen(count) >= 7) ? FONT_TEKO_22 : FONT_TEKO_24;

    int count_y = centre.y - 28;
    draw_text(gctx, count, count_font, gctx->color_value,
              GRect(body.origin.x, count_y, width, 26), GTextAlignmentCenter);

    // the rise and set times sit in the bottom corners, under the dome's feet. by day they
    // read chronologically as sunrise then sunset. at night they flip to sunset then next
    // sunrise, matching the moon's direction along the dome
    char rise[8];
    char set[8];
    fmt_clock(rise, sizeof(rise), s.rise);
    fmt_clock(set, sizeof(set), s.set);
    bool is_night = nprog >= 0;
    const char *left = is_night ? set : rise;
    const char *right = is_night ? rise : set;
    int label_y = body.origin.y + height - 16;
    draw_text(gctx, left, FONT_STM_12, gctx->color_subtitle,
              GRect(body.origin.x + PANEL_PAD, label_y, width / 2, 14), GTextAlignmentLeft);
    draw_text(gctx, right, FONT_STM_12, gctx->color_subtitle,
              GRect(body.origin.x + width / 2 - PANEL_PAD, label_y, width / 2, 14),
              GTextAlignmentRight);
}

// the header says what is being counted down to, so it flips with the tracking state
static const char *solar_label(ModuleSize size)
{
    (void)size;
    SolarTimes s = read_solar();
    bool is_sunrise = false;
    int delta = solar_next_event(s.rise, s.set, s.now, &is_sunrise);
    if (delta < 0)
    {
        return "SUN";
    }
    return is_sunrise ? "SUNRISE IN" : "SUNSET IN";
}

// --- dispatch ---

static void draw_dispatch(GridCtx *gctx)
{
    switch (gctx->size)
    {
        case MSIZE_1x2:
            draw_countdown_tile(gctx);
            break;
        case MSIZE_2x2:
            draw_arc_panel(gctx);
            break;
        default:
            break;
    }
}

// the draws already lay out against gctx->body and check gctx->headerless, so the taller
// tile grows into the extra room on its own
static void weather_solar_body(GridCtx *gctx)
{
    draw_dispatch(gctx);
}

const ModuleDef mod_weather_solar_def = {
    .label = "SUN",
    .get_label = solar_label,
    .sizes = SZ_1x2 | SZ_2x2,
    // reads the weather rise/set times and the clock (its countdown and sun progress move each
    // minute) so it must repaint on both hubs
    .features = FEATURE_WEATHER | FEATURE_TIME,
    .body = weather_solar_body
};
