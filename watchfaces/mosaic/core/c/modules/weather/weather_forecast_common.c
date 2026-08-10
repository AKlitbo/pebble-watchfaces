/**
 * @file weather_forecast_common.c
 * @brief The shared forecast-strip drawer. See the header for the shape. The sky icons come
 * from the shared icon cache (icon_get), so nothing here owns a picture and modules_cleanup's
 * icons_cleanup frees them all.
 * @ingroup mosaic_mod_weather
 */
#include "engine/grid_engine.h"
#include "weather_forecast_common.h"
#include "mosaic/draw/grid_helpers.h"
#include "text/number_format.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/icons.h"
#include "io/stores/weather_store.h"
#include "ui/weather/icons.h"
#include "settings_schema.h"
#include <stdio.h>

// draws a dotted vertical divider between each column, matching the panel border. only
// when the panel is bordered so a borderless forecast stays clean
static void draw_dividers(GridCtx *gctx, GRect body, uint8_t count)
{
    if (gctx->borderless || count < 2)
    {
        return;
    }

    graphics_context_set_stroke_color(gctx->ctx, gctx->color_accent);
    int col_w = body.size.w / count;
    for (uint8_t i = 1; i < count; i++)
    {
        int x = body.origin.x + i * col_w;
        for (int y = body.origin.y + 1; y < body.origin.y + body.size.h - 1; y += 2)
        {
            graphics_draw_pixel(gctx->ctx, GPoint(x, y));
        }
    }
}

// writes a column's temperature text, or "--" when there is no reading
void weather_fmt_temp(char *buf, size_t n, int temp, const char *fmt)
{
    if (temp == WEATHER_NO_TEMP)
    {
        snprintf(buf, n, "--");
    }
    else
    {
        snprintf(buf, n, fmt, temp);
    }
}

void weather_temp_value(int temp, char *out, size_t n, const char **unit_out)
{
    // a temperature can be genuinely negative, so only the WEATHER_NO_TEMP sentinel is "--"
    if (temp == WEATHER_NO_TEMP)
    {
        snprintf(out, n, "--");
        return;
    }

    snprintf(out, n, "%d", temp);
    *unit_out = gridlock_temp_unit_label();
}

void weather_wind_value_str(char *buf, size_t n)
{
    // the kmh<0 sentinel must not be run through gridlock_wind_value or it prints a bogus number
    int kmh = weather_store_wind_kmh();
    fmt_int_or_dash(buf, n, kmh < 0 ? -1 : gridlock_wind_value(kmh), "%d");
}

// the small forecast icon (16px) for a condition code, mirroring the shared vocabulary order.
// face-local because only this face ships the WF resources, so it stays out of the generated
// (shared) icon table that every face includes. picks the night glyph when the hourly strip
// set WX_FORECAST_NIGHT_BIT on the code
static uint32_t forecast_small_icon(uint8_t code)
{
    bool night = (code & WX_FORECAST_NIGHT_BIT) != 0;
    switch (code & ~WX_FORECAST_NIGHT_BIT)
    {
    case 0:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_CLEAR         : RESOURCE_ID_ICON_WEATHER_FC_CLEAR;
    case 1:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_PARTLY_CLOUDY : RESOURCE_ID_ICON_WEATHER_FC_PARTLY_CLOUDY;
    case 2:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_CLOUDY        : RESOURCE_ID_ICON_WEATHER_FC_CLOUDY;
    case 3:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_FOG           : RESOURCE_ID_ICON_WEATHER_FC_FOG;
    case 4:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_DRIZZLE       : RESOURCE_ID_ICON_WEATHER_FC_DRIZZLE;
    case 5:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_SLEET         : RESOURCE_ID_ICON_WEATHER_FC_SLEET;
    case 6:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_RAIN          : RESOURCE_ID_ICON_WEATHER_FC_RAIN;
    case 7:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_SLEET         : RESOURCE_ID_ICON_WEATHER_FC_SLEET;
    case 8:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_SNOW          : RESOURCE_ID_ICON_WEATHER_FC_SNOW;
    case 9:  return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_SHOWERS       : RESOURCE_ID_ICON_WEATHER_FC_SHOWERS;
    case 10: return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_SNOW_WIND     : RESOURCE_ID_ICON_WEATHER_FC_SNOW_WIND;
    case 11: return night ? RESOURCE_ID_ICON_WEATHER_FC_NIGHT_THUNDERSTORM  : RESOURCE_ID_ICON_WEATHER_FC_THUNDERSTORM;
    default: return RESOURCE_ID_ICON_WEATHER_FC_NA;
    }
}

// lays out one tight-layout day inside its own box: the label stacked over the temperature
// on the left, a small icon centred to their right. everything is placed relative to the box
static void draw_tight_box(GridCtx *gctx, GRect box, const char *label, const ForecastCell *cell)
{
    const int icon_w = 16;
    // pad the content in from the box edges so it clears the dotted dividers
    int icon_x = box.origin.x + box.size.w - icon_w - 3;
    int text_x = box.origin.x + 3;
    int text_w = icon_x - text_x;  // reaches the icon (its own margin keeps the gap) so the degree fits

    // the label sits directly over the temperature as one stack. it hugs the top in the short
    // headed body (its tuned spot) and drops toward the middle when the taller headerless body
    // gives extra room
    int extra = box.size.h > 26 ? (box.size.h - 26) / 2 : 0;
    int label_y = box.origin.y - 1 + extra;

    // day/time label on top
    graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
    graphics_draw_text(gctx->ctx, label, fonts_get(FONT_STM_12),
                       GRect(text_x, label_y, text_w, 12),
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    // temperature right under the label
    char buf[12];
    weather_fmt_temp(buf, sizeof(buf), cell->temp, "%d°");
    graphics_context_set_text_color(gctx->ctx, gctx->color_value);
    graphics_draw_text(gctx->ctx, buf, fonts_get(FONT_STM_12),
                       GRect(text_x, label_y + 12, text_w, 12),
                       GTextOverflowModeFill, GTextAlignmentLeft, NULL);

    // small icon centred vertically in the box, to the right of the text
    uint32_t res = forecast_small_icon(cell->code);
    GBitmap *bmp = icon_get(res);
    if (bmp)
    {
        GSize size = gbitmap_get_bounds(bmp).size;
        int trim_dx = 0, trim_dy = 0;
        icon_align_trim(GAlignCenter, icon_margins(res), &trim_dx, &trim_dy);
        int draw_x = icon_x + (icon_w - size.w) / 2 + trim_dx;
        int draw_y = box.origin.y + (box.size.h - size.h) / 2 + trim_dy;
        blit_tinted(gctx, bmp, GRect(draw_x, draw_y, size.w, size.h));
    }
}

// draws one row of up to four tight boxes across the given body, one column each, with the
// label stacked over the temperature on the left and a small icon to their right. used for the
// 1x4 (whole body) and for each half of the 2x4 stack
static void forecast_row_draw_tight(GridCtx *gctx, GRect body, const char labels[][FORECAST_LABEL_LEN],
                                    const ForecastCell *cells, uint8_t count)
{
    if (count > 4)
    {
        count = 4;
    }
    if (count == 0)
    {
        return;
    }

    int box_w = body.size.w / count;
    for (uint8_t i = 0; i < count; i++)
    {
        GRect box = GRect(body.origin.x + i * box_w, body.origin.y, box_w, body.size.h);
        draw_tight_box(gctx, box, labels[i], &cells[i]);
    }

    draw_dividers(gctx, body, count);
}

// the 2x4 layout: two tight four-box rows stacked, the first four columns on top and the next
// four below, so the roomier tile reads as a pair of 1x4 rows
static void forecast_row_draw_stacked(GridCtx *gctx, const char labels[][FORECAST_LABEL_LEN],
                                      const ForecastCell *cells, uint8_t count)
{
    if (count > 8)
    {
        count = 8;
    }

    GRect body = gctx->body;
    int top_h = body.size.h / 2;

    uint8_t top_count = count > 4 ? 4 : count;
    uint8_t bottom_count = count > 4 ? count - 4 : 0;

    // the bottom row only spans the width its own columns fill so a short one still lines up
    // under the top. forecast_row_draw keeps count off 0 so top_count is always at least 1
    int box_w = body.size.w / top_count;

    GRect top = GRect(body.origin.x, body.origin.y, body.size.w, top_h);
    GRect bottom = GRect(body.origin.x, body.origin.y + top_h, bottom_count * box_w,
                         body.size.h - top_h);

    forecast_row_draw_tight(gctx, top, labels, cells, top_count);
    forecast_row_draw_tight(gctx, bottom, labels + 4, cells + 4, bottom_count);
}

void forecast_row_draw(GridCtx *gctx, const char labels[][FORECAST_LABEL_LEN],
                       const ForecastCell *cells, uint8_t count)
{
    if (count == 0)
    {
        // no reading yet so show one centred placeholder instead of a blank tile
        graphics_context_set_text_color(gctx->ctx, gctx->color_subtitle);
        graphics_draw_text(gctx->ctx, "--", fonts_get(FONT_STM_14), gctx->body,
                           GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        return;
    }

    // the 1x4 is a single tight four-box row. the taller 2x4 stacks two of them for eight columns
    if (gctx->size == MSIZE_1x4)
    {
        forecast_row_draw_tight(gctx, gctx->body, labels, cells, count);
        return;
    }

    forecast_row_draw_stacked(gctx, labels, cells, count);
}
