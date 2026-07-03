/**
 * @file icon_cache.c
 * @brief Icon store. A small cache that maps a resource id to its picture and is shared
 * across a face, so each icon loads once and is freed in one place. Also carries the
 * palette tint and the auto-trim margin scan.
 */
#include "ui/icon_cache.h"
#include <string.h>

// upper bound on how many distinct icons the cache holds at once. 64 leaves plenty of
// headroom for a face's fixed icons plus its wind-direction and weather-condition glyphs
#define ICON_CACHE_MAX 64

typedef struct
{
    uint32_t    res;
    GBitmap    *bmp;
    IconMargins margin; // measured once when the icon is first cached
} IconEntry;

static IconEntry s_cache[ICON_CACHE_MAX];
static uint8_t   s_cache_count;

// paints an icon the given colour while keeping its see-through bits see-through so the
// smooth edges stay smooth. lets one white master icon take any theme colour. does
// nothing to other kinds of picture as they keep their own colours
void icon_tint(GBitmap *bmp, GColor color)
{
    if (!bmp)
    {
        return;
    }

    GColor *palette = gbitmap_get_palette(bmp);
    if (!palette)
    {
        return;
    }

    int entries;
    switch (gbitmap_get_format(bmp))
    {
        case GBitmapFormat1BitPalette: entries = 2; break;
        case GBitmapFormat2BitPalette: entries = 4; break;
        case GBitmapFormat4BitPalette: entries = 16; break;
        default: return;
    }

    for (int i = 0; i < entries; i++)
    {
        if (palette[i].a != 0)
        {
            palette[i].r = color.r;
            palette[i].g = color.g;
            palette[i].b = color.b;
        }
    }
}

// alpha 0 to 3 of a single pixel across the bitmap formats emery actually produces. a
// GColor8 keeps alpha in the top 2 bits and colour-table pixels index a GColor8 palette.
// colour-table pixels pack MSB first so the leftmost pixel sits in the high-order bits
static uint8_t px_alpha(GBitmapFormat fmt, const uint8_t *data, const GColor *palette, int x)
{
    switch (fmt)
    {
        case GBitmapFormat8Bit:
            return (data[x] >> 6) & 0x3;
        case GBitmapFormat4BitPalette:
            return palette[(data[x >> 1] >> (4 * (1 - (x & 1)))) & 0xF].a;
        case GBitmapFormat2BitPalette:
            return palette[(data[x >> 2] >> (6 - 2 * (x & 3))) & 0x3].a;
        case GBitmapFormat1BitPalette:
            return palette[(data[x >> 3] >> (7 - (x & 7))) & 0x1].a;
        default:
            // 1-bit b/w and anything else carry no alpha so treat every pixel as opaque
            return 3;
    }
}

// scans the alpha channel for the opaque bounding box and returns the transparent border
// around it. all zero for a fully transparent or unreadable icon
IconMargins icon_margins_of(GBitmap *bmp)
{
    IconMargins m = { 0, 0, 0, 0 };
#if ICON_AUTOTRIM
    if (!bmp)
    {
        return m;
    }
    GRect bounds = gbitmap_get_bounds(bmp);
    GBitmapFormat fmt = gbitmap_get_format(bmp);
    const GColor *palette = gbitmap_get_palette(bmp);

    int min_x = bounds.size.w, max_x = -1;
    int min_y = bounds.size.h, max_y = -1;
    for (int row = 0; row < bounds.size.h; row++)
    {
        GBitmapDataRowInfo info = gbitmap_get_data_row_info(bmp, bounds.origin.y + row);
        int start = info.min_x > bounds.origin.x ? info.min_x : bounds.origin.x;
        int end   = info.max_x < bounds.origin.x + bounds.size.w - 1
                        ? info.max_x : bounds.origin.x + bounds.size.w - 1;
        for (int col = start; col <= end; col++)
        {
            if (px_alpha(fmt, info.data, palette, col) != 0)
            {
                int local_x = col - bounds.origin.x;
                if (local_x < min_x) { min_x = local_x; }
                if (local_x > max_x) { max_x = local_x; }
                if (row < min_y)     { min_y = row; }
                if (row > max_y)     { max_y = row; }
            }
        }
    }

    if (max_x >= 0)
    {
        m.n = min_y;
        m.e = (bounds.size.w - 1) - max_x;
        m.s = (bounds.size.h - 1) - max_y;
        m.w = min_x;
    }
#else
    (void)bmp;
#endif
    return m;
}

GBitmap *icon_get(uint32_t res)
{
    for (uint8_t i = 0; i < s_cache_count; i++)
    {
        if (s_cache[i].res == res)
        {
            return s_cache[i].bmp;
        }
    }

    if (s_cache_count >= ICON_CACHE_MAX)
    {
        return NULL;
    }

    GBitmap *bmp = gbitmap_create_with_resource(res);
    if (!bmp)
    {
        // a bad load does not take a cache slot so a later call can try it again
        return NULL;
    }

    IconMargins margin = icon_margins_of(bmp);
#if ICON_TRIM_LOG
    APP_LOG(APP_LOG_LEVEL_DEBUG, "icon res=%lu fmt=%d N=%d E=%d S=%d W=%d",
            (unsigned long)res, (int)gbitmap_get_format(bmp), margin.n, margin.e, margin.s, margin.w);
#endif
    s_cache[s_cache_count++] = (IconEntry){ .res = res, .bmp = bmp, .margin = margin };
    return bmp;
}

IconMargins icon_margins(uint32_t res)
{
    icon_get(res); // make sure it is cached and measured
    for (uint8_t i = 0; i < s_cache_count; i++)
    {
        if (s_cache[i].res == res)
        {
            return s_cache[i].margin;
        }
    }
    return (IconMargins){ 0, 0, 0, 0 };
}

GSize icon_size(uint32_t res)
{
    GBitmap *bmp = icon_get(res);
    return bmp ? gbitmap_get_bounds(bmp).size : GSize(0, 0);
}

void icon_align_trim(GAlign align, IconMargins m, int *trim_dx, int *trim_dy)
{
    switch (align)
    {
        case GAlignTopLeft:     *trim_dx = -m.w;            *trim_dy = -m.n;            break;
        case GAlignTop:         *trim_dx = (m.e - m.w) / 2; *trim_dy = -m.n;            break;
        case GAlignTopRight:    *trim_dx = m.e;             *trim_dy = -m.n;            break;
        case GAlignLeft:        *trim_dx = -m.w;            *trim_dy = (m.s - m.n) / 2; break;
        case GAlignCenter:      *trim_dx = (m.e - m.w) / 2; *trim_dy = (m.s - m.n) / 2; break;
        case GAlignRight:       *trim_dx = m.e;             *trim_dy = (m.s - m.n) / 2; break;
        case GAlignBottomLeft:  *trim_dx = -m.w;            *trim_dy = m.s;             break;
        case GAlignBottom:      *trim_dx = (m.e - m.w) / 2; *trim_dy = m.s;             break;
        case GAlignBottomRight: *trim_dx = m.e;             *trim_dy = m.s;             break;
        default:                *trim_dx = 0;               *trim_dy = 0;               break;
    }
}

void icons_cleanup(void)
{
    for (uint8_t i = 0; i < s_cache_count; i++)
    {
        if (s_cache[i].bmp)
        {
            gbitmap_destroy(s_cache[i].bmp);
            s_cache[i].bmp = NULL;
        }
    }

    s_cache_count = 0;
}
