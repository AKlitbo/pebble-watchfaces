#include "mosaic/draw/wx_icon.h"
#include "io/stores/weather_store.h"
#include "ui/weather/icons.h"
#include <string.h>

// owns its one picture rather than going through the shared icon cache. each glyph the cache
// loads stays there until a full cache pushes it out, so a day of changing weather would hold
// several at once. sized to match the weather store's cond[32] so a long condition is not cut
// short here, which would make the strcmp miss and reload on every repaint
static GBitmap *s_bmp;
static IconMargins s_margin;
static char s_cached[32];

GBitmap *wx_icon_get(IconMargins *margin)
{
    const char *cond = weather_store_cond();
    if (!cond[0] || !strcmp(cond, "--"))
    {
        // no real condition (e.g. a failed fetch cleared the hub). drop any stale picture
        // so a module doesn't draw the last weather's icon next to a "--" readout
        wx_icon_cleanup();
        s_margin = (IconMargins){ 0, 0, 0, 0 };
        *margin = s_margin;
        return NULL;
    }

    if (strcmp(cond, s_cached) != 0)
    {
        if (s_bmp)
        {
            gbitmap_destroy(s_bmp);
            s_bmp = NULL;
        }
        s_bmp = gbitmap_create_with_resource(wx_resource_for(cond));
        // only remember the condition once the picture actually loaded, so a transient
        // allocation failure is retried on the next repaint instead of cached as a blank
        if (s_bmp)
        {
            s_margin = icon_margins_of(s_bmp);
            strncpy(s_cached, cond, sizeof(s_cached) - 1);
            s_cached[sizeof(s_cached) - 1] = '\0';
        }
        else
        {
            s_margin = (IconMargins){ 0, 0, 0, 0 };
            s_cached[0] = '\0';
        }
    }

    *margin = s_margin;
    return s_bmp;
}

void wx_icon_cleanup(void)
{
    if (s_bmp)
    {
        gbitmap_destroy(s_bmp);
        s_bmp = NULL;
    }
    s_cached[0] = '\0';
}
