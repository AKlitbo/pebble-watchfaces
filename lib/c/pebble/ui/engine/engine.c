/**
 * @file engine.c
 * @brief The skin-neutral slot engine: build one layer per slot and repaint them all on a
 * store change.
 */
#include "ui/engine/engine.h"

#include <string.h>

static Window     *s_window;
static EngineBuild s_build;
static EngineSlot  s_slots[ENGINE_MAX_SLOTS];
static Layer      *s_layers[ENGINE_MAX_SLOTS];      // draw-slots (NULL for text-slots)
static TextLayer  *s_text_layers[ENGINE_MAX_SLOTS]; // text-slots (NULL for draw-slots)
static char        s_last_text[ENGINE_MAX_SLOTS][24]; // last string set so we skip an unchanged re-fit
static uint8_t     s_count;

/**
 * @brief Draw-slot update proc: hand the module its own bounds.
 *
 * @param layer The slot layer (its data holds the slot index).
 * @param ctx The graphics context.
 */
static void draw_update(Layer *layer, GContext *ctx)
{
    uint8_t index = *(uint8_t *)layer_get_data(layer);
    if (index < s_count && s_slots[index].draw)
    {
        s_slots[index].draw(ctx, layer_get_bounds(layer), s_slots[index].data);
    }
}

/**
 * @brief Work out the current slots and make one layer per slot.
 */
static void build(void)
{
    Layer *root = window_get_root_layer(s_window);
    s_count = s_build(s_slots, ENGINE_MAX_SLOTS, layer_get_bounds(root));

    for (uint8_t i = 0; i < s_count; i++)
    {
        s_last_text[i][0] = '\0';  // force the first fit

        if (s_slots[i].zone)
        {
            // text-slot: a TextLayer with the zone's font/align/colour and fit tiers
            s_text_layers[i] = zone_make_layer(root, s_slots[i].zone);
            s_layers[i] = NULL;
        }
        else
        {
            // draw-slot: a custom layer that paints itself. a rebuild runs on every settings save
            // with the heap at its most crowded, and the create comes back NULL when it cannot
            // find the room, so the slot is left empty rather than written through a null pointer
            Layer *layer = layer_create_with_data(s_slots[i].frame, sizeof(uint8_t));
            if (layer)
            {
                *(uint8_t *)layer_get_data(layer) = i;
                layer_set_update_proc(layer, draw_update);
                layer_add_child(root, layer);
            }
            s_layers[i] = layer;
            s_text_layers[i] = NULL;
        }
    }

    engine_mark_dirty();
}

/**
 * @brief Destroy the slot layers.
 */
static void destroy(void)
{
    for (uint8_t i = 0; i < s_count; i++)
    {
        if (s_text_layers[i])
        {
            text_layer_destroy(s_text_layers[i]);
            s_text_layers[i] = NULL;
        }
        if (s_layers[i])
        {
            layer_destroy(s_layers[i]);
            s_layers[i] = NULL;
        }
    }
    s_count = 0;
}

void engine_init(Window *window, EngineBuild build_cb)
{
    s_window = window;
    s_build = build_cb;
    build();
}

void engine_deinit(void)
{
    destroy();
}

void engine_rebuild(void)
{
    destroy();
    build();
}

/**
 * @brief Repaint one slot: a text-slot re-pulls and re-fits (skipping an unchanged string), a
 * draw-slot marks its layer dirty.
 *
 * @param i The slot index.
 */
static void repaint_slot(uint8_t i)
{
    if (s_text_layers[i])
    {
        char buf[sizeof(s_last_text[0])]; // same width as the slot it gets copied into
        s_slots[i].text(buf, sizeof(buf));

        // most slots hold the same string most ticks (the date all day or the clock
        // between minutes) so skip the pixel-width fit + relayout unless it changed
        if (strcmp(buf, s_last_text[i]) != 0)
        {
            strncpy(s_last_text[i], buf, sizeof(s_last_text[i]) - 1);
            s_last_text[i][sizeof(s_last_text[i]) - 1] = '\0';

            // hand the TextLayer the persistent buffer not the local one. the layer
            // holds the pointer and doesn't copy the string
            zone_set_text_fit(s_text_layers[i], s_slots[i].zone, s_last_text[i]);
        }
    }
    else if (s_layers[i])
    {
        layer_mark_dirty(s_layers[i]);
    }
}

void engine_mark_dirty(void)
{
    // every tag set matches all-ones so this repaints the lot
    engine_mark_dirty_tags(0xFFFFFFFFu);
}

void engine_mark_dirty_tags(uint32_t changed)
{
    for (uint8_t i = 0; i < s_count; i++)
    {
        // tags == 0 means the slot declared no dependency, so repaint it on any change
        if (s_slots[i].tags == 0 || (s_slots[i].tags & changed))
        {
            repaint_slot(i);
        }
    }
}
