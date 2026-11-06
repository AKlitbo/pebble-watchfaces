#include "health_hrv.h"
#include "engine/grid_engine.h"
#include "mosaic/draw/stat_panel.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/metrics.h"
#include "text/number_format.h"
#include "io/stores/health_store.h"
#include "health/hrv_log.h"
#include "math/series.h"
#include "math/scale.h"
#include "theme/theme.h"
#include "settings_schema.h"
#include <stdio.h>
#include <time.h>

// readings are drawn against 0 to this, and the top grows to the highest reading so none clips
#define HRV_PLOT_CEIL 60
// the history covers the last two hours in five minute columns, about one burst a column
#define HRV_HISTORY_MIN 120
#define HRV_HISTORY_COLS 24

/**
 * The short name of an HRV state, for a caption or a tuning line.
 *
 * @param state An HrvState.
 * @return "STILL", "BUSY", or "WALK", or "?" for anything else.
 */
static const char *hrv_state_short(uint8_t state)
{
    static const char *const names[HRV_STATE_COUNT] = { "STILL", "BUSY", "WALK" };
    return state < HRV_STATE_COUNT ? names[state] : "?";
}

/**
 * Finds the newest reading that counts under the wearer's HRV Readings setting. Only a reading that
 * counts becomes the current one, so while there is a current reading this is it. It reads the
 * latest of each kind the log keeps, which outlasts its 24 slots, so a resting reading from before a
 * long walk still stands in.
 *
 * @param[out] ms The reading in ms.
 * @param[out] at_min When it was taken, in minutes.
 * @param[out] state Its HrvState.
 * @return True when the log holds one.
 */
static bool hrv_newest_counted(int *ms, uint32_t *at_min, uint8_t *state)
{
    const HrvLog *log = health_store_hrv_log();
    HrvCount count = (HrvCount)gridlock_hrv_count();
    bool found = false;

    for (int kind = 0; kind < HRV_STATE_COUNT; kind++)
    {
        int kind_ms = 0;
        uint32_t kind_at = 0;
        if (hrv_log_counts((HrvState)kind, count) && hrv_log_last(log, (HrvState)kind, &kind_ms, &kind_at) &&
            (!found || kind_at > *at_min))
        {
            *ms = kind_ms;
            *at_min = kind_at;
            *state = (uint8_t)kind;
            found = true;
        }
    }
    return found;
}

/**
 * Writes how long ago a reading was taken, such as "NOW", "8M", or "2H", short enough for a
 * caption beside a value.
 *
 * @param out Where the text goes.
 * @param n The room in out.
 * @param at_min When the reading was taken, in minutes.
 */
static void hrv_age(char *out, size_t n, uint32_t at_min)
{
    uint32_t now_min = (uint32_t)time(NULL) / 60;
    uint32_t age = now_min > at_min ? now_min - at_min : 0;

    if (age < 1)
    {
        snprintf(out, n, "NOW");
    }
    else if (age < 60)
    {
        snprintf(out, n, "%luM", (unsigned long)age);
    }
    else
    {
        snprintf(out, n, "%luH", (unsigned long)(age / 60));
    }
}

/**
 * The reading the main panels show. The current one when there is one, else the newest reading
 * that counts under the wearer's HRV Readings setting, else the newest of any kind, so a panel is
 * never blank once a single burst has landed.
 *
 * @param after Where to write what goes after the value: "MS" for the current reading, its age for
 * an older one that counts, and "WLK" or "BSY" for one the setting leaves out.
 * @param n The room in after.
 * @return The reading in ms, or -1 with none at all.
 */
static int hrv_shown(char *after, size_t n)
{
    int ms = health_store_hrv_ms();
    if (ms >= 0)
    {
        snprintf(after, n, "MS");
        return ms;
    }

    uint32_t at = 0;
    uint8_t state = 0;
    if (hrv_newest_counted(&ms, &at, &state))
    {
        hrv_age(after, n, at);
        return ms;
    }

    uint8_t flags = 0;
    if (hrv_log_get(health_store_hrv_log(), 0, &ms, &at, &flags))
    {
        snprintf(after, n, "%s", (flags & HRV_LOG_STATE_MASK) == HRV_STATE_WALKING ? "WLK" : "BSY");
        return ms;
    }

    snprintf(after, n, "MS");
    return -1;
}

/**
 * The top of a plot, which holds at HRV_PLOT_CEIL unless a reading goes higher.
 *
 * @param highest The highest reading in the plot.
 * @return The value the top of the plot stands for.
 */
static int hrv_plot_top(int highest)
{
    return highest > HRV_PLOT_CEIL ? highest : HRV_PLOT_CEIL;
}

/**
 * How tall a bar for a reading is, at least a pixel so a low reading still shows.
 *
 * @param ms The reading.
 * @param top The value the top of the plot stands for.
 * @param h The plot's height.
 * @return The bar's height.
 */
static int hrv_bar_h(int ms, int top, int h)
{
    int bar = (ms * h) / top;
    if (bar < 1)
    {
        bar = 1;
    }
    return bar > h ? h : bar;
}

// --- HRV (1x2) ---

static void hrv_value_1x2(char *out, size_t n, const char **unit_out)
{
    static char after[8];
    fmt_int_or_dash(out, n, hrv_shown(after, sizeof(after)), "%d");
    *unit_out = after;
}

static const StatPanel1x2 hrv_desc = {
    .value_1x2 = hrv_value_1x2,
    .icon = &ICON_HEART,
    .font = FONT_TEKO_26,
};

// borrow the Heart Rate colour so the heart tiles read as one family
STAT_PANEL_1X2(health_hrv, "HRV", SZ_1x2, FEATURE_HEALTH, MOD_HEALTH_HEARTRATE, &hrv_desc);

// --- HRV trend (1x2) ---

/**
 * Fills a small triangle pointing up or down, as the trend mark beside the value.
 *
 * @param gctx The grid context.
 * @param tip Where the point of the triangle goes.
 * @param up True to point up.
 */
static void hrv_draw_arrow(GridCtx *gctx, GPoint tip, bool up)
{
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_icon);
    // a triangle five rows tall, drawn as shrinking lines so no path has to be allocated
    for (int row = 0; row < 5; row++)
    {
        int y = up ? tip.y + row : tip.y - row;
        graphics_draw_line(gctx->ctx, GPoint(tip.x - row, y), GPoint(tip.x + row, y));
    }
}

static void health_hrv_trend_body(GridCtx *gctx)
{
    int ms = health_store_hrv_ms();
    int low = -1;
    int avg = -1;
    int high = -1;
    hrv_log_today(health_store_hrv_log(), &low, &avg, &high);

    char val[8];
    char after[8];
    fmt_int_or_dash(val, sizeof(val), hrv_shown(after, sizeof(after)), "%d");
    gh_stat_1x2(gctx, val, after, FONT_TEKO_26, NULL);

    // an arrow only means something against the current reading, not an older one. the average
    // is of resting readings only, so a walking reading that counts under Any Time gets none either
    int counted_ms = 0;
    uint32_t counted_at = 0;
    uint8_t state = 0;
    bool walking = hrv_newest_counted(&counted_ms, &counted_at, &state) && state == HRV_STATE_WALKING;
    if (ms < 0 || avg < 0 || walking)
    {
        return;
    }

    // the average small on the right, with the arrow above it saying which side of it the
    // latest reading is. level with the average draws no arrow. it sits on the same line as the
    // unit after the reading, which on a Pebble Time 2 is 5px up from the body's bottom edge
    char avgbuf[16];
    snprintf(avgbuf, sizeof(avgbuf), "AVG %d", avg);
    int line_h = metric_for(FONT_STM_12)->line_h;
    GRect row = GRect(gctx->body.origin.x + PANEL_PAD, gctx->body.origin.y + gctx->body.size.h - line_h - 5,
                      gctx->body.size.w - EDGE_PAD(gctx) - PANEL_PAD, line_h);
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, avgbuf, fonts_get(FONT_STM_12), row, GTextOverflowModeFill,
                       GTextAlignmentRight, NULL);

    if (ms != avg)
    {
        int tip_x = gctx->body.origin.x + gctx->body.size.w - EDGE_PAD(gctx) - 6;
        int tip_y = ms > avg ? gctx->body.origin.y + 4 : gctx->body.origin.y + 8;
        hrv_draw_arrow(gctx, GPoint(tip_x, tip_y), ms > avg);
    }
}

const ModuleDef mod_health_hrv_trend_def = {
    .label = "HRV",
    .sizes = SZ_1x2,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_hrv_trend_body
};

// --- Heart (2x2) ---

/**
 * Draws the last hour of heart rate as a line across area, scaled to the hour's own low and high
 * so its shape shows. A minute with no reading breaks the line.
 *
 * @param gctx The grid context.
 * @param area Where the line goes.
 * @param history The last hour, a reading a minute, with 0 for none.
 * @param lo The hour's lowest reading.
 * @param hi The hour's highest reading.
 */
static void heart_draw_spark(GridCtx *gctx, GRect area, const uint8_t *history, int lo, int hi)
{
    // a flat hour still draws as a line through the middle rather than a divide by nothing
    int floor = lo - 3;
    int top = hi + 3;
    bool have_prev = false;
    GPoint prev = GPointZero;

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_value);
    for (int minute = 0; minute < 60; minute++)
    {
        int bpm = history[minute];
        if (bpm <= 0)
        {
            have_prev = false;
            continue;
        }

        int x = area.origin.x + (minute * (area.size.w - 1)) / 59;
        int y = plot_y(area.origin.y, area.size.h, floor, top, clamp_int(bpm, floor, top));
        GPoint point = GPoint(x, y);
        if (have_prev)
        {
            graphics_draw_line(gctx->ctx, prev, point);
        }
        else
        {
            graphics_draw_pixel(gctx->ctx, point);
        }
        prev = point;
        have_prev = true;
    }
}

// where the glyphs really sit, measured on a Pebble Time 2. Teko 34's digits start 12px below
// where they are drawn and stand 22px tall. Share Tech Mono 12's capitals start 4px below and
// stand 8px tall. the font metrics include design lift, so laying out from them leaves uneven
// gaps. the body also runs 2px under the border on the left and 1px on the right. the bottom row
// sits a pixel nearer its edge than the gap says, which reads as even under the heavier header
#define HEART_PAD 4
#define HEART_DIGIT_DROP 12
#define HEART_DIGIT_H 22
#define HEART_CAPTION_DROP 4
#define HEART_CAPTION_H 8
#define HEART_INSET_LEFT 2
#define HEART_INSET_RIGHT 1
#define HEART_INSET_BOTTOM 0

/**
 * One row of the heart tile: a reading big on the left and two small grey lines on the right, the
 * upper one level with the top of the digits and the lower one level with their bottom, so the
 * row reads as one block.
 *
 * @param gctx The grid context.
 * @param digit_top Where the top of the digits lands.
 * @param value The reading.
 * @param upper The grey line level with the top of the digits.
 * @param lower The grey line level with their bottom.
 */
static void heart_row(GridCtx *gctx, int digit_top, const char *value, const char *upper, const char *lower)
{
    int left = gctx->body.origin.x + HEART_INSET_LEFT + HEART_PAD;
    int width = gctx->body.size.w - HEART_INSET_LEFT - HEART_INSET_RIGHT - HEART_PAD * 2;
    int caption_h = metric_for(FONT_STM_12)->line_h;
    int big_h = metric_for(FONT_TEKO_34)->line_h;

    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, value, fonts_get(FONT_TEKO_34),
                       GRect(left, digit_top - HEART_DIGIT_DROP, width, big_h), GTextOverflowModeFill,
                       GTextAlignmentLeft, NULL);

    int lower_top = digit_top + HEART_DIGIT_H - HEART_CAPTION_H - HEART_CAPTION_DROP;
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, upper, fonts_get(FONT_STM_12),
                       GRect(left, digit_top - HEART_CAPTION_DROP, width, caption_h), GTextOverflowModeFill,
                       GTextAlignmentRight, NULL);
    graphics_draw_text(gctx->ctx, lower, fonts_get(FONT_STM_12), GRect(left, lower_top, width, caption_h),
                       GTextOverflowModeFill, GTextAlignmentRight, NULL);
}

/**
 * Heart rate and HRV in one tile. The heart rate on top with the last hour's low and high, the hour
 * as a line, and HRV along the bottom drawn the same way, with what the wearer was doing for it.
 * Both rows sit HEART_PAD from the tile's edges, so the gaps all round match.
 */
static void health_heart_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    GRect body = gctx->body;

    // the heart rate, with the last hour's low and high
    const uint8_t *history = health_store_hr_history();
    int lo = 0;
    int hi = 0;
    int last = 0;
    int valid = series_range(history, 60, 0, &lo, &hi, &last);
    int live = health_store_hr();
    char bpm[8];
    char range[12] = "";
    fmt_int_or_dash(bpm, sizeof(bpm), live > 0 ? live : (valid > 0 ? last : -1), "%d");
    if (valid > 0)
    {
        snprintf(range, sizeof(range), "%d-%d", lo, hi);
    }
    int top_digits = body.origin.y + HEART_PAD;
    heart_row(gctx, top_digits, bpm, "BPM", range);

    // HRV, with what the wearer was doing for it, or its age when it is an older reading standing
    // in for the current one
    char hrv[8];
    char after[8];
    char kind[8] = "";
    int shown = hrv_shown(after, sizeof(after));
    fmt_int_or_dash(hrv, sizeof(hrv), shown, "%d");
    if (health_store_hrv_ms() < 0)
    {
        snprintf(kind, sizeof(kind), "%s", shown >= 0 ? after : "");
    }
    else
    {
        // the kind of the reading on screen, which is the newest that counts rather than the
        // newest logged, since a burst that does not count is logged too
        int ms = 0;
        uint32_t at = 0;
        uint8_t state = 0;
        if (hrv_newest_counted(&ms, &at, &state))
        {
            snprintf(kind, sizeof(kind), "%s", hrv_state_short(state));
        }
    }
    int bottom_digits = body.origin.y + body.size.h - HEART_INSET_BOTTOM - HEART_PAD - HEART_DIGIT_H;
    heart_row(gctx, bottom_digits, hrv, "HRV MS", kind);

    // the hour as a line in the gap between the two rows, with the same gap above and below it
    int spark_top = top_digits + HEART_DIGIT_H + HEART_PAD;
    int spark_bottom = bottom_digits - HEART_PAD;
    GRect spark = GRect(body.origin.x + HEART_INSET_LEFT + HEART_PAD, spark_top,
                        body.size.w - HEART_INSET_LEFT - HEART_INSET_RIGHT - HEART_PAD * 2, spark_bottom - spark_top);
    if (valid > 0 && spark.size.h > 2)
    {
        heart_draw_spark(gctx, spark, history, lo, hi);
    }
}

const ModuleDef mod_health_heart_def = {
    .label = "HEART",
    .sizes = SZ_2x2,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_heart_body
};

// --- Night HRV (1x2) ---

static void hrv_night_value(char *out, size_t n, const char **unit_out)
{
    fmt_int_or_dash(out, n, hrv_log_night_avg(health_store_hrv_log()), "%d");
    *unit_out = "MS";
}

static const StatPanel1x2 hrv_night_desc = {
    .value_1x2 = hrv_night_value,
    .icon = &ICON_SNOOZE,
    .font = FONT_TEKO_26,
};

// borrow the Sleep colour, since it is a night figure
STAT_PANEL_1X2(health_hrv_night, "NIGHT HRV", SZ_1x2, FEATURE_HEALTH, MOD_HEALTH_SLEEP, &hrv_night_desc);

// --- HRV history (2x2) ---

static void health_hrv_history_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    const HrvLog *log = health_store_hrv_log();
    uint32_t now_min = (uint32_t)time(NULL) / 60;
    int ms = 0;
    uint32_t at = 0;
    uint8_t flags = 0;

    // the latest reading big across the top, its age small on the right
    char val[8];
    char age[12] = "";
    bool any = hrv_log_get(log, 0, &ms, &at, &flags);
    fmt_int_or_dash(val, sizeof(val), any ? ms : -1, "%d");
    gh_graph_value_top(gctx, val);
    if (any)
    {
        hrv_age(age, sizeof(age), at);
        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        GRect row = GRect(gctx->body.origin.x + PANEL_PAD, gctx->body.origin.y,
                          gctx->body.size.w - PANEL_PAD * 2, 14);
        graphics_draw_text(gctx->ctx, age, fonts_get(FONT_STM_12), row, GTextOverflowModeFill,
                           GTextAlignmentRight, NULL);
    }

    GRect plot = gctx->body;
    plot.origin.x += PANEL_PAD;
    plot.size.w -= PANEL_PAD * 2;
    plot.origin.y += 28;
    plot.size.h -= 30;

    // the baseline, so an empty stretch still reads as a chart with nothing in it
    graphics_context_set_stroke_color(gctx->ctx, gctx->color_subtitle);
    int base_y = plot.origin.y + plot.size.h;
    graphics_draw_line(gctx->ctx, GPoint(plot.origin.x, base_y), GPoint(plot.origin.x + plot.size.w - 1, base_y));

    int highest = 0;
    for (int back = 0; hrv_log_get(log, back, &ms, &at, &flags); back++)
    {
        // a clock set back leaves a reading after now, which the bars draw as the newest, so it
        // counts here the same way
        uint32_t ago = now_min > at ? now_min - at : 0;
        if (ago < HRV_HISTORY_MIN && ms > highest)
        {
            highest = ms;
        }
    }
    int top = hrv_plot_top(highest);
    int col_w = plot.size.w / HRV_HISTORY_COLS;

    for (int back = 0; hrv_log_get(log, back, &ms, &at, &flags); back++)
    {
        uint32_t ago = now_min > at ? now_min - at : 0;
        if (ago >= HRV_HISTORY_MIN)
        {
            break;
        }

        // newest on the right, one column per five minutes
        int col = HRV_HISTORY_COLS - 1 - (int)(ago * HRV_HISTORY_COLS / HRV_HISTORY_MIN);
        int h = hrv_bar_h(ms, top, plot.size.h);
        GRect bar = GRect(plot.origin.x + col * col_w, base_y - h, col_w > 1 ? col_w - 1 : 1, h);

        // a still or busy reading is in the value colour and a walking one in the caption grey, so
        // the resting readings stand out and the walking ones still show where they fell. a bar is
        // only 2px wide, too narrow for an outline to look any different from a fill
        bool walking = (flags & HRV_LOG_STATE_MASK) == HRV_STATE_WALKING;
        graphics_context_set_fill_color(gctx->ctx, walking ? gctx->color_subtitle : gctx->color_value);
        graphics_fill_rect(gctx->ctx, bar, 0, GCornerNone);
    }
}

const ModuleDef mod_health_hrv_history_def = {
    .label = "HRV 2H",
    .sizes = SZ_2x2,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_hrv_history_body
};

// --- Still vs moving (2x2) ---

static void health_hrv_states_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x2)
    {
        return;
    }

    const HrvLog *log = health_store_hrv_log();
    int row_h = gctx->body.size.h / HRV_STATE_COUNT;
    int caption_h = metric_for(FONT_STM_12)->line_h;
    int value_h = metric_for(FONT_TEKO_22)->line_h;

    // the ages line up in a column just past the widest label, so they read down as one list
    GSize label_w = graphics_text_layout_get_content_size("STILL", fonts_get(FONT_STM_12), gctx->body,
                                                           GTextOverflowModeFill, GTextAlignmentLeft);
    int age_x = gctx->body.origin.x + PANEL_PAD + label_w.w + 6;

    for (int state = 0; state < HRV_STATE_COUNT; state++)
    {
        int ms = -1;
        uint32_t at = 0;
        char age[12] = "";
        char val[8];
        bool found = hrv_log_last(log, (HrvState)state, &ms, &at);

        if (found)
        {
            hrv_age(age, sizeof(age), at);
        }
        fmt_int_or_dash(val, sizeof(val), found ? ms : -1, "%d");

        // the kind on the left, its age in the column beside it, and the reading on the right
        int top = gctx->body.origin.y + state * row_h;
        int caption_top = top + (row_h - caption_h) / 2;
        GRect label = GRect(gctx->body.origin.x + PANEL_PAD, caption_top, label_w.w + 6, caption_h);
        GRect age_box = GRect(age_x, caption_top, gctx->body.size.w - (age_x - gctx->body.origin.x), caption_h);
        GRect right = GRect(gctx->body.origin.x + PANEL_PAD, top + (row_h - value_h) / 2 - 3,
                            gctx->body.size.w - PANEL_PAD - EDGE_PAD(gctx), value_h);

        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        graphics_draw_text(gctx->ctx, hrv_state_short((uint8_t)state), fonts_get(FONT_STM_12), label, GTextOverflowModeFill,
                           GTextAlignmentLeft, NULL);
        graphics_draw_text(gctx->ctx, age, fonts_get(FONT_STM_12), age_box, GTextOverflowModeFill,
                           GTextAlignmentLeft, NULL);
        graphics_context_set_text_color(gctx->ctx, gctx->color_value);
        graphics_draw_text(gctx->ctx, val, fonts_get(FONT_TEKO_22), right, GTextOverflowModeFill,
                           GTextAlignmentRight, NULL);
    }
}

const ModuleDef mod_health_hrv_states_def = {
    .label = "HRV",
    .sizes = SZ_2x2,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_hrv_states_body
};

// --- HRV today (1x4, 2x4) ---

/**
 * Draws today's hours as 24 bars into area, the hour now in the icon colour so it stands out.
 *
 * @param gctx The grid context.
 * @param area Where the bars go.
 * @param log The HRV log to read the hours from.
 */
static void hrv_draw_day(GridCtx *gctx, GRect area, const HrvLog *log)
{
    time_t now = time(NULL);
    int hour_now = localtime(&now)->tm_hour;
    int highest = 0;

    for (int hour = 0; hour < HRV_LOG_HOURS; hour++)
    {
        int avg = hrv_log_hour_avg(log, hour);
        if (avg > highest)
        {
            highest = avg;
        }
    }

    int top = hrv_plot_top(highest);
    int col_w = area.size.w / HRV_LOG_HOURS;
    int base_y = area.origin.y + area.size.h;

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_line(gctx->ctx, GPoint(area.origin.x, base_y), GPoint(area.origin.x + area.size.w - 1, base_y));

    for (int hour = 0; hour < HRV_LOG_HOURS; hour++)
    {
        int avg = hrv_log_hour_avg(log, hour);
        if (avg < 0)
        {
            continue;
        }

        int h = hrv_bar_h(avg, top, area.size.h);
        GRect bar = GRect(area.origin.x + hour * col_w, base_y - h, col_w > 1 ? col_w - 1 : 1, h);
        graphics_context_set_fill_color(gctx->ctx, hour == hour_now ? gctx->color_icon : gctx->color_value);
        graphics_fill_rect(gctx->ctx, bar, 0, GCornerNone);
    }
}

static void health_hrv_day_body(GridCtx *gctx)
{
    const HrvLog *log = health_store_hrv_log();
    GRect area = gctx->body;
    area.origin.x += PANEL_PAD;
    area.size.w -= PANEL_PAD * 2;

    if (gctx->size == MSIZE_1x4)
    {
        area.origin.y += 3;
        area.size.h -= 6;
        hrv_draw_day(gctx, area, log);
        return;
    }

    if (gctx->size != MSIZE_2x4)
    {
        return;
    }

    // the day's low, average, and high and the latest reading across the top, the hours below
    int low = -1;
    int avg = -1;
    int high = -1;
    hrv_log_today(log, &low, &avg, &high);
    static const char *const labels[4] = { "LOW", "AVG", "HIGH", "NOW" };
    const int values[4] = { low, avg, high, health_store_hrv_ms() };
    int stats_h = gh_stat_row4(gctx, area, labels, values);
    area.origin.y += stats_h;
    area.size.h -= stats_h + 3;
    hrv_draw_day(gctx, area, log);
}

const ModuleDef mod_health_hrv_day_def = {
    .label = "HRV TODAY",
    .sizes = SZ_1x4 | SZ_2x4,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_hrv_day_body
};

// --- HRV tuning (2x4) ---

// the tuning table's rows. Share Tech Mono gives every character the same width, so fixed width
// fields line up as columns. rows sit 11px apart, since the capitals stand 8px tall
#define TUNING_PITCH 11

/**
 * Draws one line of the tuning table.
 *
 * @param gctx The grid context.
 * @param cap_top Where the top of its capitals lands.
 * @param text The line.
 * @param color Its colour.
 */
static void tuning_line(GridCtx *gctx, int cap_top, const char *text, GColor color)
{
    int left = gctx->body.origin.x + HEART_INSET_LEFT + HEART_PAD;
    int width = gctx->body.size.w - HEART_INSET_LEFT - HEART_INSET_RIGHT - HEART_PAD * 2;
    graphics_context_set_text_color(gctx->ctx, color);
    graphics_draw_text(gctx->ctx, text, fonts_get(FONT_STM_12),
                       GRect(left, cap_top - HEART_CAPTION_DROP, width, metric_for(FONT_STM_12)->line_h),
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

/**
 * Writes the share of a burst's gaps that went into its reading, or "--" when there were none.
 *
 * @param out Where the text goes.
 * @param n The room in out.
 * @param kept The differences that went into the reading.
 * @param gaps The gaps the burst heard.
 */
static void tuning_kept(char *out, size_t n, int kept, int gaps)
{
    if (gaps <= 0)
    {
        snprintf(out, n, "--");
        return;
    }

    snprintf(out, n, "%d%%", (kept * 100 + gaps / 2) / gaps);
}

/**
 * Shows the numbers a wearer tunes their settings from, as a table: the burst being measured on
 * the first row, live, then the latest bursts with their movement score, steps, what they counted
 * as, the reading, and the share of beats that went into it. The limit in use is along the bottom.
 * A bright row counted for the main reading, and a grey one did not. Bright is white or black,
 * whichever reads on the theme's background, since the small text is hard to read in a colour.
 */
static void health_hrv_tuning_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x4)
    {
        return;
    }

    int top = gctx->body.origin.y + HEART_PAD;
    int bottom = gctx->body.origin.y + gctx->body.size.h - HEART_INSET_BOTTOM - HEART_PAD - HEART_CAPTION_H;
    char line[48];
    GColor bright = gcolor_legible_over(theme_background(settings_u8(SETTING_THEME)));

    tuning_line(gctx, top, "TIME   MOV STP KIND  HRV KEPT", gctx->color_subtitle);

    int row = top + TUNING_PITCH;
    int score = -1;
    int steps = -1;
    int held = 0;
    if (health_store_hrv_live(&score, &steps, &held))
    {
        char held_text[8];
        snprintf(held_text, sizeof(held_text), "%dS", held);
        HrvState state = hrv_log_state_of(score, steps, health_store_hrv_still_mg(), health_store_hrv_walk_steps());
        snprintf(line, sizeof(line), "%-5s %4d %3d %-5s %3s %4s", held_text, score, steps, hrv_state_short(state), "--", "--");
        tuning_line(gctx, row, line, bright);
    }
    else
    {
        tuning_line(gctx, row, "WAITING FOR A BURST", gctx->color_subtitle);
    }

    // as many bursts as fit above the limits line
    for (int back = 0; row + TUNING_PITCH * 2 <= bottom; back++)
    {
        HrvBurst burst;
        if (!health_store_hrv_recent(back, &burst))
        {
            break;
        }

        time_t at = (time_t)burst.at_sec;
        struct tm *local = localtime(&at);
        char result[8];
        char kept[8];
        fmt_int_or_dash(result, sizeof(result), burst.ms, "%d");
        tuning_kept(kept, sizeof(kept), burst.kept, burst.gaps);
        snprintf(line, sizeof(line), "%02d:%02d %4d %3d %-5s %3s %4s", local->tm_hour, local->tm_min, burst.score,
                 burst.steps, hrv_state_short(burst.state), result, kept);

        row += TUNING_PITCH;
        tuning_line(gctx, row, line, burst.counted ? bright : gctx->color_subtitle);
    }

    snprintf(line, sizeof(line), "STILL<=%d  WALK>=%d STEPS", health_store_hrv_still_mg(), health_store_hrv_walk_steps());
    tuning_line(gctx, bottom, line, gctx->color_subtitle);
}

const ModuleDef mod_health_hrv_tuning_def = {
    .label = "HRV TUNING",
    .sizes = SZ_2x4,
    .features = FEATURE_HEALTH,
    .theme_alias = MOD_HEALTH_HEARTRATE,
    .body = health_hrv_tuning_body
};
