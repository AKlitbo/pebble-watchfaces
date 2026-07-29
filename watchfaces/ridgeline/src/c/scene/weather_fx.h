/**
 * @file weather_fx.h
 * @brief What the weather adds to the scene: clouds, falling precipitation, fog banks, a
 * lightning fork, and snow on the peaks.
 *
 * The phone sends a condition token, fx_recipe turns it into a recipe, and scene.c draws
 * the parts in its own z-order (clouds sit behind the ridges, rain falls in front of them).
 * Splitting it this way keeps the recipe table in one place, so retuning a condition means
 * editing one row rather than hunting through the draw code.
 *
 * @ingroup watchface-ridgeline
 */
#pragma once
#include <pebble.h>

#include "theme/theme.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

/** @brief How much cloud is in the sky. */
typedef enum
{
    CLOUDS_NONE,      ///< Open sky
    CLOUDS_DRIFTING,  ///< One cloud, parked over the disc
    CLOUDS_OVERCAST   ///< A band of puffs running right across the sky
} CloudCover;

/** @brief How much of the sun or moon the clouds leave showing. */
typedef enum
{
    CEL_FULL,   ///< Open sky, the disc draws in full
    CEL_PEEK,   ///< A cloud is parked over it, so it only half shows
    CEL_HIDDEN  ///< Overcast: no disc at all
} CelestialVis;

/** @brief What is falling out of the sky, if anything. */
typedef enum
{
    PRECIP_NONE,
    PRECIP_DROPS,  ///< Fine dots for drizzle
    PRECIP_DASHES, ///< Slanted strokes for rain
    PRECIP_FLAKES, ///< Little crosses for snow
    PRECIP_MIXED   ///< Dots and strokes together for sleet and freezing rain
} PrecipKind;

/** @brief Everything one condition adds to the scene. */
typedef struct SkyRecipe
{
    uint8_t clouds;    ///< A CloudCover
    uint8_t celestial; ///< A CelestialVis
    uint8_t precip;    ///< A PrecipKind
    uint8_t drops;     ///< How many particles fall
    bool    fog;       ///< Lay fog banks over the ridges
    bool    bolt;      ///< Fork a bolt out of the cloud
    bool    snowcap;   ///< Put snow along the crests
} SkyRecipe;

/**
 * @brief Reads the phone's condition token into a recipe.
 *
 * The trailing "_NIGHT" marker is ignored: night is decided from sunrise and sunset, so the
 * same sky reads the same either way. An empty or unrecognised token gives plain open sky.
 *
 * @param condition The condition abbreviation, e.g. "RAIN" or "PCLDY_NIGHT".
 * @return The recipe to draw.
 */
SkyRecipe fx_recipe(const char *condition);

/**
 * @brief Draw the recipe's clouds. One cloud parks over the disc, a second sits across the sky.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param recipe The recipe to draw.
 * @param disc Where the sun or moon is, so a peeking cloud lands on it.
 */
void fx_draw_clouds(GContext *ctx, const Palette *pal, const SkyRecipe *recipe, GPoint disc);

/**
 * @brief Draw the falling precipitation as a curtain across the sky.
 *
 * Particle positions come from the minute rather than a random number, so the scene shifts
 * once a minute on the tick that is already happening and never costs a timer.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param recipe The recipe to draw.
 * @param minute The clock minute, which shuffles the particles.
 */
void fx_draw_precip(GContext *ctx, const Palette *pal, const SkyRecipe *recipe, int minute);

/**
 * @brief Draw fog banks lying across the ridges.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
void fx_draw_fog(GContext *ctx, const Palette *pal);

/**
 * @brief Draw a lightning fork out of the cloud.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 */
void fx_draw_bolt(GContext *ctx, const Palette *pal);

/** @} */
