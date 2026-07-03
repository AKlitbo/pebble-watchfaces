/**
 * @file engine.h
 * @brief The skin-neutral slot engine: builds one layer per slot from a face's slot list
 * and repaints them all when a store changes. A slot is either a text-slot (a TextLayer built
 * from a Zone, so the font-fit tiers still apply) or a draw-slot (a custom layer whose
 * update_proc paints a gauge, icon, or panel). A face supplies a build callback. A dynamic
 * face (the grid) re-runs it on a layout change. This is the shared core the shell and the
 * modular grid engine both sit on.
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

#include "ui/zone.h"

/** @addtogroup lib @{ */

// most slots a single face can declare (modular's grid tops out at 10 cells and the shell
// faces at ~12 zones + widgets)
#define ENGINE_MAX_SLOTS 16

/**
 * @brief One slot of a face. Set `zone` (+ `text`) for a text-slot, or `frame` (+ `draw`)
 * for a draw-slot. Never both.
 */
typedef struct
{
    GRect frame;                                            /**< draw-slot layer frame */
    void (*draw)(GContext *ctx, GRect bounds, const void *data); /**< draw-slot paint */
    const void *data;                                       /**< draw-slot payload */

    const Zone *zone;                                       /**< text-slot geometry + font tiers */
    void (*text)(char *out, size_t n);                      /**< text-slot: pull + format from a store */
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

/** @} */
