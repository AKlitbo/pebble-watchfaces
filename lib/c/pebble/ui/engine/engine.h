/**
 * @file engine.h
 * @brief The skin-neutral slot engine: builds one layer per slot from a face's slot list
 * and repaints them all when a store changes. A slot is either a text-slot (a TextLayer built
 * from a Zone, so the font-fit tiers still apply) or a draw-slot (a custom layer whose
 * update_proc paints a gauge, icon, or panel). A face supplies a build callback. A dynamic
 * face (the grid) re-runs it on a layout change. This is the shared core a face's own engine
 * sits on.
 *
 * @ingroup lib_ui
 */
#pragma once
#include <pebble.h>

#include "ui/zone.h"

/**
 * @addtogroup lib_ui
 * @{
 */

/// Most slots a single face can declare. Ten is the ceiling of the largest layout a face builds
/// here, so the slot arrays hold no dead entries. A face wanting more zones bumps this
#define ENGINE_MAX_SLOTS 10

/**
 * @brief One slot of a face. Set `zone` (+ `text`) for a text-slot, or `frame` (+ `draw`)
 * for a draw-slot. Never both.
 */
typedef struct
{
    GRect frame;                                            /**< Draw slot layer frame */
    void (*draw)(GContext *ctx, GRect bounds, const void *data); /**< Draw slot paint */
    const void *data;                                       /**< Draw slot data */

    const Zone *zone;                                       /**< Text slot area and font tiers */
    void (*text)(char *out, size_t n);                      /**< Text slot: pulls and formats from a store */

    uint32_t tags;                                          /**< Which stores this slot reads from, for engine_mark_dirty_tags (0 means repaint on any change) */
} EngineSlot;

/**
 * @brief Fills `out` with the current slots and returns the count. A static face returns a
 * fixed list. A dynamic face works it out from settings. `bounds` is the window's root
 * bounds, handy for a full-window draw-slot (overlays that paint at absolute coords).
 */
typedef uint8_t (*EngineBuild)(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Build the slot layers for a face and paint them once.
 *
 * @param window The main window.
 * @param build The face's slot-builder.
 */
void engine_init(Window *window, EngineBuild build);

/** @brief Tear down the slot layers. */
void engine_deinit(void);

/** @brief Rebuild the slot layers (after a layout or theme change). */
void engine_rebuild(void);

/** @brief Repaint every slot: text-slots re-pull + fit, draw-slots mark dirty. */
void engine_mark_dirty(void);

/**
 * @brief Repaint only the slots whose tags intersect `changed` (plus any slot with no tags).
 * Lets a face route a store change to just the slots that read from it.
 *
 * @param changed The dependency bits that changed.
 */
void engine_mark_dirty_tags(uint32_t changed);

/** @} */
