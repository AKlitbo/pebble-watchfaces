/**
 * @file stat_panel.h
 * @brief A data-driven drawer for the thin single-value stat panels. Most 1x2 (and some 2x2
 * goal) panels are the same shape: read one store value, format it, hand it to gh_stat_1x2 (or
 * gh_stat_goal_2x2 on the 2x2 goal half). This collapses that repeated body into one shared
 * drawer plus a small per-module descriptor, so a panel is just its value formatting and a
 * table entry. The STAT_PANEL macros still emit the per-module mod_<name>_def symbol, so the
 * catalog switch, the persisted ModuleType ids, and the vibrant tables are untouched.
 *
 * A 1x2-only panel needs just the four StatPanel1x2 fields. A goal panel that also draws a 2x2
 * nests that as its `base` and adds the 2x2 half, so a one-size panel carries no dead 2x2 slots.
 *
 * @ingroup mosaic_draw
 */
#pragma once
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/engine/catalog.h"

/**
 * @addtogroup mosaic_draw
 * @{
 */

/**
 * @brief The 1x2 half of a stat panel: how to format its value and which icon and unit to draw.
 */
typedef struct
{
    /// Format the 1x2 value into out. Set *unit_out to the trailing unit label when there is one
    /// (it starts NULL), so a unit from the same reading is not worked out twice
    void (*value_1x2)(char *out, size_t n, const char **unit_out);
    const IconSpec *icon;      /**< Static right-hand icon, or NULL for none or a dynamic one */
    IconSpec (*icon_fn)(void); /**< Dynamic icon worked out from the value (wind direction). NULL means use icon */
    FontId font;               /**< The 1x2 value font */
} StatPanel1x2;

/**
 * @brief A stat panel that also draws a 2x2 goal tile. The 1x2 half is `base`; the rest is the
 * goal path.
 */
typedef struct
{
    StatPanel1x2 base;                       /**< The 1x2 half, reused as is */
    void (*value_2x2)(char *out, size_t n);  /**< The big value. NULL reuses base.value_1x2 */
    void (*goal_2x2)(char *goal_out, size_t n, int *value, int *goal); /**< Goal string and progress */
    const char *goal_caption;                /**< "GOAL" or "LIMIT" under the 2x2 value */
} StatPanelDesc;

/**
 * @brief Draws the 1x2 stat panel: value + optional unit on the left, icon on the right.
 *
 * @param gctx The grid draw context.
 * @param desc The 1x2 descriptor.
 */
void stat_panel_draw_1x2(GridCtx *gctx, const StatPanel1x2 *desc);

/**
 * @brief Draws a goal panel: the 1x2 half via stat_panel_draw_1x2, the 2x2 half as a goal tile.
 *
 * @param gctx The grid draw context.
 * @param desc The goal descriptor.
 */
void stat_panel_draw_goal(GridCtx *gctx, const StatPanelDesc *desc);

// emit the small body wrapper (the header and headerless looks share it) and the module def
// keeping the per-module mod_<name>_def symbol the catalog switch already references
#define STAT_PANEL_1X2(name, LABEL, SIZES, FEAT, ALIAS, DESCPTR)                      \
    static void name##_body(GridCtx *gctx) { stat_panel_draw_1x2(gctx, (DESCPTR)); }  \
    const ModuleDef mod_##name##_def = {                                              \
        .label = (LABEL), .sizes = (SIZES), .features = (FEAT),                       \
        .theme_alias = (ALIAS), .body = name##_body }

#define STAT_PANEL_GOAL(name, LABEL, SIZES, FEAT, ALIAS, DESCPTR)                      \
    static void name##_body(GridCtx *gctx) { stat_panel_draw_goal(gctx, (DESCPTR)); }  \
    const ModuleDef mod_##name##_def = {                                               \
        .label = (LABEL), .sizes = (SIZES), .features = (FEAT),                        \
        .theme_alias = (ALIAS), .body = name##_body }

/** @} */
