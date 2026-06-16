// painted overlays: battery gauge, bar labels, weather/heart/feet glyphs
#include "widgets.h"

#include "layout.h"
#include "settings/settings.h"
#include "theme/theme.h"
#include "draw/draw.h"
#include "weather/icons.h"

static Layer *s_labels_layer;   // WEATHER / HEARTRATE / STEPS labels
static Layer *s_battery_layer;  // battery gauge in the top-left block
static Layer *s_glyphs_layer;   // weather / heart / feet icons over the readouts

static GFont s_font_label;  // Antonio 10 - bar labels
static GBitmap *s_heart_bmp, *s_feet_bmp, *s_wx_bmp, *s_thermo_bmp;
static uint32_t s_wx_resource;  // resource id currently in s_wx_bmp, to skip redundant reloads
static int s_batt_level;

typedef struct
{
    GRect area;
    const char *text;
} LcarsLabel;

// paint the four LCARS bar labels
static void labels_update_proc(Layer *layer, GContext *ctx)
{
    const LcarsLabel labels[] = {
        {LBL_STARDATE, "STARDATE"},
        {LBL_WEATHER, "SENSORS"},
        {LBL_HR, "VITALS"},
        {LBL_STEPS, "TRAVERSAL"},
    };

    for (unsigned i = 0; i < ARRAY_LENGTH(labels); i++)
    {
        draw_lcars_label(ctx, labels[i].area, labels[i].text, s_font_label, LCARS_LABEL_COLOR,
                         3);
    }
}

// battery icon drawn on top of the top-left LCARS block
static void battery_update_proc(Layer *layer, GContext *ctx)
{
    GColor accent = panel_accent_for_theme(g_settings.Theme);
    draw_lcars_battery_gauge(ctx, GRect(6, 4, 32, 12), s_batt_level, accent);
}

// blit a transparent icon bitmap, skipping nulls
static void draw_icon(GContext *ctx, GBitmap *bmp, GRect r)
{
    if (!bmp)
    {
        return;
    }
    
    // honor PNG transparency
    graphics_context_set_compositing_mode(ctx, GCompOpSet); 
     
    graphics_draw_bitmap_in_rect(ctx, bmp, r);
}

// paint the weather / thermometer / heart / feet glyphs
static void glyphs_update_proc(Layer *layer, GContext *ctx)
{
    draw_icon(ctx, s_wx_bmp, WX_ICON);
    draw_icon(ctx, s_thermo_bmp, THERMO_ICON);
    draw_icon(ctx, s_heart_bmp, HR_ICON);
    draw_icon(ctx, s_feet_bmp, FEET_ICON);
}

// create the overlay layers, label font, and icon bitmaps
void widgets_create(Layer *parent)
{
    GRect bounds = layer_get_bounds(parent);

    s_font_label = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ANTONIO_10));

    s_heart_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_HEART);
    s_feet_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_FEET);
    s_thermo_bmp = gbitmap_create_with_resource(RESOURCE_ID_ICON_THERMOMETER);
    s_wx_resource = RESOURCE_ID_ICON_WI_NA;
    s_wx_bmp = gbitmap_create_with_resource(s_wx_resource);

    s_battery_layer = layer_create(bounds);
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(parent, s_battery_layer);

    s_labels_layer = layer_create(bounds);
    layer_set_update_proc(s_labels_layer, labels_update_proc);
    layer_add_child(parent, s_labels_layer);

    s_glyphs_layer = layer_create(bounds);
    layer_set_update_proc(s_glyphs_layer, glyphs_update_proc);
    layer_add_child(parent, s_glyphs_layer);
}

// destroy the overlay layers, font, and icon bitmaps
void widgets_destroy(void)
{
    layer_destroy(s_labels_layer);
    layer_destroy(s_battery_layer);
    layer_destroy(s_glyphs_layer);

    if (s_heart_bmp)
    {
        gbitmap_destroy(s_heart_bmp);
    }

    if (s_feet_bmp)
    {
        gbitmap_destroy(s_feet_bmp);
    }

    if (s_thermo_bmp)
    {
        gbitmap_destroy(s_thermo_bmp);
    }

    if (s_wx_bmp)
    {
        gbitmap_destroy(s_wx_bmp);
    }

    fonts_unload_custom_font(s_font_label);
}

// set the battery level (0..100) and redraw the gauge
void widgets_set_battery(int level)
{
    s_batt_level = level;
    if (s_battery_layer)
    {
        layer_mark_dirty(s_battery_layer);
    }
}

// swap the weather glyph for the given condition
void widgets_set_weather_icon(const char *condition)
{
    uint32_t resource = wx_resource_for(condition);

    // the ~30-min refresh and every temp-unit re-request resend the same
    // condition, so skip the destroy/create + repaint when the icon is unchanged
    if (resource == s_wx_resource && s_wx_bmp)
    {
        return;
    }

    if (s_wx_bmp)
    {
        gbitmap_destroy(s_wx_bmp);
        s_wx_bmp = NULL;
    }

    s_wx_resource = resource;
    s_wx_bmp = gbitmap_create_with_resource(resource);

    if (s_glyphs_layer)
    {
        layer_mark_dirty(s_glyphs_layer);
    }
}

// repaint the bar labels and the theme-colored battery gauge
void widgets_mark_labels_dirty(void)
{
    if (s_labels_layer)
    {
        layer_mark_dirty(s_labels_layer);
    }

    // the battery's healthy fill is theme-colored, so repaint it on theme change
    if (s_battery_layer)
    {
        layer_mark_dirty(s_battery_layer);
    }
}
