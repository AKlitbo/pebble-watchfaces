/**
 * @file layout.c
 * @brief The face's chrome, its pennant, and the slot list it hands the engine.
 *
 * Nothing here is baked: the field, the stipple, the panel and the pointer are all drawn, so a
 * colour change is a repaint rather than a resource swap.
 *
 * @ingroup watchface-sidereel
 */
#include "layout.h"

#include "band/band.h"
#include "dev/dev.h"
#include "draw/fonts.h"
#include "draw/sprockets.h"
#include "draw/text.h"
#include "mosaic/draw/header_fonts.h"
#include "engine/panel.h"
#include "layout_string.h"
#include "io/stores/system_store.h"
#include "io/stores/time_store.h"
#include "ui/icon_cache.h"
#include "reel/reel.h"
#include "settings_schema.h"
#include "system/settings/settings.h"
#include "theme/theme.h"
#include "ui/fonts.h"

/**
 * @addtogroup watchface-sidereel
 * @{
 */

// the pennant outline in the layer's own coordinates: a rectangular body with a triangular tip
// whose apex sits on the layer's mid-line, which is CENTRE_Y once the layer is placed. the body
// runs to CELL_X + CELL_W so the taper starts on the same edge the panels above and below end on.
//
// the left edge is named one pixel out because gpath_draw_filled insets its edges by a pixel, so
// CELL_X - 1 is what actually paints from CELL_X and lines the pointer up with the panels above
// and below it. the perforation column starts on CELL_X for the same reason: filling from a pixel
// further out left the four rows above that column standing proud of the rest of the edge
static const GPathInfo PENNANT_PATH = {
    .num_points = 5,
    .points = (GPoint[]){{CELL_X - 1, 0},
                         {CELL_X + CELL_W, 0},
                         {125, PENNANT_H / 2},
                         {CELL_X + CELL_W, PENNANT_H - 1},
                         {CELL_X - 1, PENNANT_H - 1}},
};

static GPath *s_pennant;

// held so a colour change can re-tint the window under the slots as well as the slots themselves
static Window *s_window;

/** @brief One drawn cell: which module it holds and what footprint it has. */
typedef struct
{
    uint8_t module;
    uint8_t size;  // a ModuleSize
} ResolvedCell;

// resolved once per build so each slot's data pointer has somewhere stable to point
static ResolvedCell s_cells[CELL_COUNT];

/**
 * @brief Register one custom font, saying so when it does not load.
 *
 * fonts_get quietly hands back a system font for a slot that holds nothing, which is the right
 * behaviour on the screen but hides the cause: the face just renders one line in the wrong
 * typeface. This face loads fifteen cuts and they come out of the same heap the bitmaps do, so a
 * later one failing is a real possibility worth hearing about.
 *
 * @param id The slot to register under.
 * @param res The font resource.
 */
static void load_font(FontId id, uint32_t res)
{
    GFont handle = fonts_load_custom_font(resource_get_handle(res));

    if (!handle)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "font slot %d did not load, falling back to a system font", (int)id);
    }

    fonts_register(id, handle);
}

static GFont   s_header_font;
static uint8_t s_header_choice = 0xFF;

void sidereel_apply_header_font(void)
{
    uint8_t choice = settings_u8(SETTING_HEADER_FONT);
    if (choice == s_header_choice)
    {
        return;
    }

    if (s_header_font)
    {
        fonts_unload_custom_font(s_header_font);
    }

    s_header_font = fonts_load_custom_font(resource_get_handle(header_font_spec(choice)->resource));
    fonts_register(FONT_HEADER, s_header_font);
    s_header_choice = choice;
}

static void load_fonts(void)
{
    load_font(FONT_CLOCK_56, RESOURCE_ID_FONT_CLOCK_56);

    // the whole set gridlock's panels name: this face has both the 1x2 and the 2x2, and
    // the big-digit panels reach for the largest of these in a 2x2
    load_font(FONT_TEKO_96, RESOURCE_ID_FONT_TEKO_96);
    load_font(FONT_TEKO_88, RESOURCE_ID_FONT_TEKO_88);
    load_font(FONT_TEKO_72, RESOURCE_ID_FONT_TEKO_72);
    load_font(FONT_TEKO_54, RESOURCE_ID_FONT_TEKO_54);
    load_font(FONT_TEKO_46, RESOURCE_ID_FONT_TEKO_46);
    load_font(FONT_TEKO_34, RESOURCE_ID_FONT_TEKO_34);
    load_font(FONT_TEKO_26, RESOURCE_ID_FONT_TEKO_26);
    load_font(FONT_TEKO_24, RESOURCE_ID_FONT_TEKO_24);
    load_font(FONT_TEKO_22, RESOURCE_ID_FONT_TEKO_22);
    load_font(FONT_STM_14, RESOURCE_ID_FONT_STM_14);
    load_font(FONT_STM_12, RESOURCE_ID_FONT_STM_12);

    // FONT_HEADER is the one slot the user picks, so it is loaded through its own swap rather
    // than pinned here. apply_header_font runs again on every settings push
    sidereel_apply_header_font();

    // the system-font fallbacks the panel bodies reach for when a custom size overflows
    fonts_register_system(FONT_TIME_SM, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    fonts_register_system(FONT_DATE_SM, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    fonts_register_system(FONT_DATE_XS, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    fonts_register_system(FONT_COORD, fonts_get_system_font(FONT_KEY_GOTHIC_14));
}

/**
 * @brief The static chrome: the left field and the perforated strip.
 *
 * The reel paints its own panel and highlight, because those sit under the digits in the same
 * layer and would otherwise be a repaint behind them.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_chrome(GContext *ctx, GRect bounds, const void *data)
{
    const Chrome *chrome = theme_chrome();

    graphics_context_set_fill_color(ctx, chrome->background);
    graphics_fill_rect(ctx, LEFT_FIELD, 0, GCornerNone);

    // the strip carries the panel colour, so the reel reads as running on past the field rather
    // than as a second thing sat beside it
    sprockets_draw(ctx, SPROCKET_STRIP, chrome->panel, chrome->background);
}

/**
 * @brief The day track around the outside.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the full window).
 * @param data Unused.
 */
static void draw_band(GContext *ctx, GRect bounds, const void *data)
{
    band_draw(ctx, bounds, theme_chrome());
}

/**
 * @brief Draw one status glyph inside the pointer, tinted to sit on its fill.
 *
 * @param ctx The graphics context.
 * @param res The icon resource.
 * @param at Where to put it, in the pointer's own coordinates.
 * @param tint The colour to paint it.
 */
static void draw_pennant_icon(GContext *ctx, uint32_t res, GRect at, GColor tint)
{
    GBitmap *glyph = icon_get(res);

    if (!glyph)
    {
        return;
    }

    icon_tint(glyph, tint);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, glyph, at);
}

/**
 * @brief The hour pointer: a filled pennant with the hour in it, tip on the centred minute.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the pennant frame).
 * @param data Unused.
 */
static void draw_pennant(GContext *ctx, GRect bounds, const void *data)
{
    const Chrome *chrome = theme_chrome();

    if (s_pennant)
    {
        graphics_context_set_fill_color(ctx, chrome->pointer);
        gpath_draw_filled(ctx, s_pennant);

        // the tail is screened with the panels' own dither and perforated like the strip beside
        // the reel, so the pointer reads as a length of film rather than a plain wedge. both sit
        // inside the straight part of the body, well left of where the taper starts
        panel_checker(ctx, PENNANT_TAIL, chrome->background);
        sprockets_draw(ctx, PENNANT_PERF, chrome->pointer, chrome->background);
    }

    char hour[4];
    side_format_hour(hour, sizeof(hour), time_store_tm()->tm_hour);

    // the same cut the reel runs on, sat right of the glyphs rather than centred on the whole
    // shape, so the hour and the minute read as one clock across the bar
    graphics_context_set_text_color(ctx, chrome->pointer_ink);
    side_draw_centred(ctx, hour, FONT_CLOCK_56, PENNANT_TEXT);

    // the two status glyphs ride the pointer's left edge, each shown only if its own toggle asks
    // for it. neither reads as an alert: a glyph that only appeared on the bad state would be one,
    // so both report their state either way and the pointer looks the same whichever it is.
    //
    // the link is lit when the phone is there and slashed when it drops, which is what the Show
    // Connection Icon toggle says it does on every other face
    if (settings_u8(SETTING_BLUETOOTH_ICON))
    {
        draw_pennant_icon(ctx, system_store_bluetooth() ? RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_ON
                                                        : RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_OFF,
                          PENNANT_BT, chrome->pointer_ink);
    }

    // and the same for quiet time: a muted speaker while it holds and a full one when sound is
    // back on, which is the pair gridlock's Quiet Time panel draws
    if (settings_u8(SETTING_QUIET_TIME_ICON))
    {
        draw_pennant_icon(ctx, dev_force_quiet() || quiet_time_is_active()
                                   ? RESOURCE_ID_ICON_SYSTEM_VOLUME_MUTED
                                   : RESOURCE_ID_ICON_SYSTEM_VOLUME_FULL,
                          PENNANT_QUIET, chrome->pointer_ink);
    }
}

/**
 * @brief A panel cell: hands its chosen module the cell and lets the shared chrome draw it.
 *
 * @param ctx The graphics context.
 * @param bounds The slot bounds (the cell).
 * @param data Points at the cell's entry in s_cell_module.
 */
static void draw_cell(GContext *ctx, GRect bounds, const void *data)
{
    const ResolvedCell *cell = data;

    panel_draw(ctx, bounds, (ModuleType)cell->module, (ModuleSize)cell->size);
}

// a module's store dependencies as engine repaint tags. an empty cell still needs a non-zero tag,
// or it would repaint on every frame of a scroll
static uint32_t tags_for(ModuleFeature features)
{
    uint32_t tags = 0;

    tags |= (features & FEATURE_TIME) ? TAG_TIME : 0;
    tags |= (features & FEATURE_WEATHER) ? TAG_WEATHER : 0;
    tags |= (features & FEATURE_HEALTH) ? TAG_HEALTH : 0;
    tags |= (features & FEATURE_SYSTEM) ? TAG_SYSTEM : 0;

    return tags ? tags : TAG_CHROME;
}

void sidereel_setup(Window *window)
{
    s_window = window;

    theme_refresh();
    load_fonts();

    window_set_background_color(window, theme_chrome()->background);

    s_pennant = gpath_create(&PENNANT_PATH);
}

uint8_t sidereel_build(EngineSlot *out, uint8_t max, GRect bounds)
{
    uint8_t index = 0;

    // chrome first so it sits under everything
    out[index++] = (EngineSlot){.frame = bounds, .draw = draw_chrome,
                                .tags = TAG_CHROME};

    out[index++] = (EngineSlot){.frame = REEL_PANEL, .draw = reel_draw,
                                .tags = TAG_REEL | TAG_TIME};

    // the pennant after the reel, so its tip covers the digits it points at
    out[index++] = (EngineSlot){.frame = PENNANT_FRAME, .draw = draw_pennant,
                                .tags = TAG_TIME | TAG_SYSTEM};

    // one slot per placed block. the parser has already dropped anything that would not fit, so
    // whatever is left here can be laid out without further checking
    uint8_t placed = sidereel_block_count();

    for (uint8_t i = 0; i < placed && index < max - 1; i++)
    {
        SidereelBlock block = sidereel_block(i);

        s_cells[i].module = block.module;
        s_cells[i].size = block.height >= 2 ? MSIZE_2x2 : MSIZE_1x2;

        out[index++] = (EngineSlot){
            .frame = GRect(CELL_X, ROW_TOP(block.row), CELL_W,
                           block.height >= 2 ? CELL_H_2X : CELL_H),
            .draw = draw_cell,
            .data = &s_cells[i],
            .tags = tags_for(module_def(s_cells[i].module)->features),
        };
    }

    // the day track goes on last, so it frames everything else rather than being covered by the
    // reel panel it overlaps at the top and bottom edges
    out[index++] = (EngineSlot){.frame = bounds, .draw = draw_band,
                                .tags = TAG_CHROME | TAG_TIME | TAG_WEATHER | TAG_SYSTEM};

    return index > max ? max : index;
}

void sidereel_apply_theme(void)
{
    theme_refresh();

    if (s_window)
    {
        window_set_background_color(s_window, theme_chrome()->background);
    }
}

void sidereel_teardown(void)
{
    panel_cleanup();

    s_window = NULL;

    if (s_pennant)
    {
        gpath_destroy(s_pennant);
        s_pennant = NULL;
    }

    fonts_unload_all();
}

/** @} */
