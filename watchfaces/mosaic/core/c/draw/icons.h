/**
 * @file icons.h
 * @brief The face's icon draw helpers. The low-level cache (load-once by resource id, the
 * palette tint, and the auto-trim margins) lives in the shared lib (ui/icon_cache.h). This
 * header adds the Gridlock-specific bits on top: the GridCtx-aware draw helpers and the named
 * specs that carry each icon's own little placement nudge.
 *
 * An icon's drawing does not always sit dead-centre in its picture. IconSpec.dx/dy is
 * the fix for that and it is added on top of the placement offset. That way a caller
 * only says where it wants the icon and does not worry about how the art is padded.
 * Unmeasured nudges sit at 0 until they are checked in the emulator.
 *
 * @ingroup mosaic_draw
 */
#pragma once
#include <pebble.h>
#include "engine/grid_engine.h"
#include "ui/icon_cache.h"   // shared low-level cache: icon_get/size/margins/tint/cleanup

/**
 * @addtogroup mosaic_draw
 * @{
 */

/**
 * @brief An icon resource together with its own little placement nudge.
 */
typedef struct
{
    uint32_t res;
    int8_t   dx;
    int8_t   dy;
} IconSpec;

// per-icon taste nudge on top of the shared auto-trim. flip to 0 to zero every IconSpec
// dx/dy at once (with auto-trim on, 0 gives pure art-driven placement, the clean baseline).
// the old dx/dy also compensated for whitespace, which auto-trim now owns, so leave this 0
// until the values are re-tuned as small taste offsets
#define ICON_NUDGES 0
#if ICON_NUDGES
#define ICON_DX(spec) ((spec)->dx)
#define ICON_DY(spec) ((spec)->dy)
#else
#define ICON_DX(spec) 0
#define ICON_DY(spec) 0
#endif

// fixed icons with one resource each. the nudges all start at 0
// fill them in once you have checked them in the emulator then drop the matching offset
extern const IconSpec ICON_HEART;
extern const IconSpec ICON_UV;
extern const IconSpec ICON_RAIN;
extern const IconSpec ICON_FEET;
extern const IconSpec ICON_DISTANCE;
extern const IconSpec ICON_FIRE;
extern const IconSpec ICON_SNOOZE;
extern const IconSpec ICON_TIME_LATE;
extern const IconSpec ICON_THERMOMETER;
extern const IconSpec ICON_CLOCK;
extern const IconSpec ICON_DATE_TIME;
extern const IconSpec ICON_GLOBE;
extern const IconSpec ICON_HUMIDITY;
extern const IconSpec ICON_SUNRISE;
extern const IconSpec ICON_SUNSET;
extern const IconSpec ICON_WIND;
extern const IconSpec ICON_BLUETOOTH;
extern const IconSpec ICON_BLUETOOTH_SLASH;

/**
 * @brief Draws a named icon, pinned in the body with an extra placement offset.
 *
 * The icon's own nudge (spec.dx/dy) is added to dx/dy for you.
 *
 * @param gctx The grid context.
 * @param spec The icon spec.
 * @param align Where to pin it in the body.
 * @param dx Extra sideways offset.
 * @param dy Extra up and down offset.
 */
void icon_draw(GridCtx *gctx, const IconSpec *spec, GAlign align, int dx, int dy);

/**
 * @brief Draws a plain icon resource for the wind-direction and weather-condition
 * icons that do not have a fixed spec, pinned in the body with a placement offset.
 *
 * @param gctx The grid context.
 * @param res The resource id.
 * @param align Where to pin it in the body.
 * @param dx Sideways offset.
 * @param dy Up and down offset.
 */
void icon_draw_res(GridCtx *gctx, uint32_t res, GAlign align, int dx, int dy);

/**
 * @brief Draws an icon at an exact spot you give it, for callers that work out the
 * position themselves. For example lining an icon's bottom up with some text.
 *
 * @param gctx The grid context.
 * @param res The resource id.
 * @param dst The box to draw it in.
 */
void icon_draw_rect(GridCtx *gctx, uint32_t res, GRect dst);

/**
 * @brief Visible width of an icon: its art minus the transparent margins on each side.
 *
 * @param res The resource id.
 * @return The opaque width in pixels.
 */
int icon_visible_width(uint32_t res);

/**
 * @brief Turns a compass direction like "N" or "NE" into the matching wind-arrow icon.
 * Falls back to the plain wind icon when the direction is empty or not recognised.
 *
 * @param dir The compass direction.
 * @return The resource id.
 */
uint32_t icon_wind_dir(const char *dir);

/**
 * @brief The wind-arrow icon for a compass direction packaged as an IconSpec,
 * carrying the same placement nudge as the plain wind icon (ICON_WIND).
 *
 * @param dir The compass direction (see icon_wind_dir).
 * @return An IconSpec for the matching arrow.
 */
IconSpec icon_wind_spec(const char *dir);

/**
 * @brief Tints a bitmap with the context's icon colour and draws it into @p dst
 * with the standard see-through compositing. The one place the tint + GCompOpSet
 * + draw sequence lives.
 *
 * @param gctx The grid context (supplies the draw context and icon colour).
 * @param bmp The picture to draw (nothing happens if it is NULL).
 * @param dst The box to draw it in.
 */
void blit_tinted(GridCtx *gctx, GBitmap *bmp, GRect dst);

/** @} */
