/**
 * @file layout.c
 * @brief ide-vscode-emery face: the chosen desktop's layout table bound to shared readouts, its
 * baked frame, and the overlays draw-slot (battery, weather glyph, Quiet Time, and bluetooth).
 * No shell. The face drives the engine directly from main.
 */
#include "layout.h"

#include "ui/fonts.h"
#include "ui/zone.h"
#include "draw/fonts.h"
#include "ui/readouts.h"
#include "ui/icon_cache.h"
#include "theme/theme.h"
#include "widgets/widgets.h"
#include "settings_schema.h"
#include "io/stores/system_store.h"
#include "io/stores/weather_store.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"

// the table the screen draws from. a copy, so the VS Code theme can paint its colours over it and
// the engine's zones point somewhere that outlives the rebuild
static DesktopLayout s_layout;

// 0xFF = nothing loaded yet, which forces the first load
static uint8_t s_loaded_desktop = 0xFF;

/**
 * @brief Paint a VS Code theme's colours over the Dark+ ones in the table.
 *
 * @param layout The active layout to paint.
 * @param theme The theme setting value.
 */
static void apply_theme_colors(DesktopLayout *layout, uint8_t theme)
{
    GColor stats = stats_color_for_theme(theme);

    layout->zones[ZONE_TIME].color = time_color_for_theme(theme);
    layout->zones[ZONE_DATE].color = date_color_for_theme(theme);
    layout->zones[ZONE_COND].color = stats;
    layout->zones[ZONE_WEATHER].color = stats;
    layout->zones[ZONE_HR].color = hr_color_for_theme(theme);
    layout->zones[ZONE_STEPS].color = steps_color_for_theme(theme);
    layout->wx_color = stats;
    layout->batt_color = stats;
    layout->batt_fill = stats;
    layout->status_color = stats;
}

/**
 * @brief Paint the Terminal look's greens over a desktop's Classic colours.
 *
 * Every desktop's Terminal frame is green on black, so one set of greens serves them all: the
 * brightest for the clock and the values, a softer one for the date, and darker ones for a status
 * that is off.
 *
 * @param layout The active layout to paint.
 */
static void apply_terminal_colors(DesktopLayout *layout)
{
    for (uint8_t zone = 0; zone < ZONE_COUNT; zone++)
    {
        layout->zones[zone].color = GColorGreen;
    }

    layout->zones[ZONE_DATE].color = GColorScreaminGreen;
    layout->wx_color = GColorGreen;
    layout->batt_color = GColorGreen;
    layout->batt_fill = GColorGreen;
    layout->status_color = GColorGreen;
    layout->status_off_color = GColorIslamicGreen;
    layout->quiet_color = GColorMintGreen;
}

/**
 * @brief Load a desktop's fonts into their slots, freeing whatever the last desktop loaded.
 *
 * A desktop that sets two jobs in the same resource gets one loaded font in both slots, which the
 * registry frees once.
 *
 * @param layout The desktop's layout table.
 */
static void load_fonts(const DesktopLayout *layout)
{
    fonts_unload_all();

    bool loaded[FONT_COUNT] = {false};

    for (uint8_t slot = 0; slot < FONT_COUNT; slot++)
    {
        // a built-in font belongs to the firmware, so it is parked rather than loaded and freed
        if (layout->system_fonts[slot])
        {
            fonts_register_system(slot, fonts_get_system_font(layout->system_fonts[slot]));
            loaded[slot] = true;
            continue;
        }

        uint32_t resource = layout->fonts[slot];
        if (!resource)
        {
            continue;
        }

        // a slot sharing a font that failed to load stays empty too. fonts_get hands back the
        // system font for a failed slot, and registering that as the face's own would free a
        // firmware font on the way out
        bool shared = false;
        for (uint8_t earlier = 0; earlier < slot; earlier++)
        {
            if (layout->fonts[earlier] == resource)
            {
                if (loaded[earlier])
                {
                    fonts_register(slot, fonts_get(earlier));
                    loaded[slot] = true;
                }

                shared = true;
                break;
            }
        }

        if (!shared)
        {
            loaded[slot] = fonts_load(slot, resource);
        }
    }
}

// --- Baked frame ---
static BitmapLayer *s_frame_layer;
static GBitmap *s_frame_bitmap;
static uint32_t s_loaded_frame;

/**
 * @brief (Re)load the baked frame, freeing the old one first so only one is ever held.
 *
 * @param resource The frame's resource id.
 */
static void load_frame(uint32_t resource)
{
    if (resource == s_loaded_frame && s_frame_bitmap)
    {
        return;
    }

    s_loaded_frame = resource;

    if (s_frame_bitmap)
    {
        gbitmap_destroy(s_frame_bitmap);
        s_frame_bitmap = NULL;
    }

    s_frame_bitmap = gbitmap_create_with_resource(resource);

    if (s_frame_layer)
    {
        bitmap_layer_set_bitmap(s_frame_layer, s_frame_bitmap);
    }
}

/**
 * @brief Overlays draw-slot: a clock drawn a glyph at a time, the battery, the weather condition
 * glyph, and the Quiet Time and bluetooth status.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_overlays(GContext *ctx, GRect bounds, const void *data)
{
    widgets_draw_spaced_clock(ctx, &s_layout);
    widgets_draw_weather(ctx, &s_layout, weather_store_cond());
    widgets_draw_status(ctx, &s_layout, system_store_battery(), system_store_bluetooth());
}

void vscode_apply_theme(void)
{
    uint8_t desktop = vscode_desktop();
    uint8_t theme = settings_u8(SETTING_THEME);
    const DesktopLayout *table = desktop_layout(desktop);

    s_layout = *table;

    uint32_t frame = table->background;
    if (desktop == DESKTOP_VSCODE)
    {
        apply_theme_colors(&s_layout, theme);
        frame = bg_resource_for_theme(theme);
    }
    else if (vscode_desktop_theme() == DESKTOP_THEME_TERMINAL)
    {
        apply_terminal_colors(&s_layout);
        frame = table->background_terminal;
    }

    // the fonts only change with the desktop, and the engine rebuild that follows makes the
    // text layers again, so none of them is left holding a freed font
    if (desktop != s_loaded_desktop)
    {
        load_fonts(table);
        s_loaded_desktop = desktop;
    }

    load_frame(frame);
}

const DesktopLayout *vscode_layout(void)
{
    return &s_layout;
}

void vscode_setup(Window *window)
{
    Layer *root = window_get_root_layer(window);
    window_set_background_color(window, GColorBlack);

    // the frame bitmap sits at the bottom under the engine's slot layers
    s_frame_layer = bitmap_layer_create(layer_get_bounds(root));
    layer_add_child(root, bitmap_layer_get_layer(s_frame_layer));

    vscode_apply_theme();
}

// the build below writes all seven slots without reading max, so the engine's cap has to hold them
// a slot added below raises this count too
_Static_assert(ENGINE_SLOTS_MAX >= 7, "vscode_build writes seven slots");

/**
 * @brief The text function for a zone, the desktop's own when it has one.
 *
 * @param zone The zone id.
 * @param plain The framework readout the zone uses otherwise.
 * @return The function to run.
 */
static ReadoutText text_for(uint8_t zone, ReadoutText plain)
{
    // a clock drawn a glyph at a time is painted by the overlays slot, so its text slot stays empty
    if (zone == ZONE_TIME && s_layout.clock_pitch)
    {
        return NULL;
    }

    return s_layout.text[zone] ? s_layout.text[zone] : plain;
}

uint8_t vscode_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t i = 0;

    // overlays first so they sit under the text readouts
    out[i++] = (EngineSlot){.frame = bounds, .draw = draw_overlays};

    out[i++] = (EngineSlot){.zone = &s_layout.zones[ZONE_TIME],    .text = text_for(ZONE_TIME, readout_time)};
    out[i++] = (EngineSlot){.zone = &s_layout.zones[ZONE_DATE],    .text = text_for(ZONE_DATE, readout_date)};
    out[i++] = (EngineSlot){.zone = &s_layout.zones[ZONE_COND],    .text = text_for(ZONE_COND, readout_weather_cond)};
    out[i++] = (EngineSlot){.zone = &s_layout.zones[ZONE_WEATHER], .text = text_for(ZONE_WEATHER, readout_weather_temp)};
    out[i++] = (EngineSlot){.zone = &s_layout.zones[ZONE_HR],      .text = text_for(ZONE_HR, readout_hr)};
    out[i++] = (EngineSlot){.zone = &s_layout.zones[ZONE_STEPS],   .text = text_for(ZONE_STEPS, readout_steps)};

    return i;
}

void vscode_teardown(void)
{
    icons_cleanup();
    bitmap_layer_destroy(s_frame_layer);
    s_frame_layer = NULL;

    if (s_frame_bitmap)
    {
        gbitmap_destroy(s_frame_bitmap);
        s_frame_bitmap = NULL;
    }

    fonts_unload_all();

    // force a reload if the window is recreated
    s_loaded_desktop = 0xFF;
    s_loaded_frame = 0;
}
