/**
 * @file catalog.h
 * @brief The module catalog. Each module is just data: a label plus little functions
 * that make its value and subtitle and maybe paint an icon or take over the whole
 * panel. The engine draws them. Modules never talk to services on their own.
 *
 * @ingroup gridlock_engine
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup gridlock_engine
 * @{
 */

/**
 * @brief Every module type. The order gets saved to storage, so only ever add to the end.
 */
typedef enum
{
    MOD_EMPTY,
    MOD_COMPOSITE_DATETIME,
    MOD_SYSTEM_BATTERY,
    MOD_WEATHER_CURRENT,
    MOD_WEATHER_TEMPERATURE,
    MOD_HEALTH_HEARTRATE,
    MOD_HEALTH_STEPS,
    MOD_HEALTH_CALORIES,
    MOD_HEALTH_SLEEP,
    MOD_HEALTH_ACTIVITY,
    MOD_WEATHER_HUMIDITY,
    MOD_WEATHER_WIND,
    MOD_WEATHER_SUNRISE,
    MOD_WEATHER_SUNSET,
    MOD_WEATHER_DAYLIGHT,
    MOD_WEATHER_CONDITIONS,
    MOD_TIME_BEATS,
    MOD_TIME_DATE,
    MOD_TIME_CLOCK,
    MOD_TIME_ZONE_1,
    MOD_TIME_ANALOG,
    MOD_WEATHER_UV,
    MOD_WEATHER_HILO,
    MOD_WEATHER_PRECIP,
    MOD_HEALTH_DISTANCE,
    MOD_WEATHER_FORECAST_HOURLY,
    MOD_WEATHER_FORECAST_DAILY,
    MOD_WEATHER_NOW,
    MOD_WEATHER_SOLAR,
    MOD_STOCK_QUOTE,
    MOD_STOCK_WATCHLIST,
    MOD_SYSTEM_CONNECTION,
    MOD_HEALTH_HR_GRAPH,
    MOD_TIME_MOON,
    MOD_SYSTEM_QUIET,
    MOD_WEATHER_FEELS_LIKE,
    MOD_WEATHER_PRESSURE,
    MOD_WEATHER_DEW_POINT,
    MOD_TIME_DATETIME,  ///< Retired slot, kept so the ids after it never shift
    MOD_HEALTH_STEPS_GRAPH,
    MOD_CALENDAR_COUNTDOWN,
    MOD_CALENDAR_AGENDA,
    MOD_CALENDAR_FREEBUSY,
    MOD_TIME_WEEKNUM,
    MOD_TIME_DAYOFYEAR,
    MOD_TIME_EPOCH,
    MOD_TIME_YEAREND,
    MOD_TIME_WEEKDAY_DOTS,
    MOD_TIME_MOON_COUNTDOWN,
    MOD_TIME_MONTH,
    MOD_TIME_JULIAN,
    MOD_TIME_WEEKSLEFT,
    MOD_SYSTEM_STATUS,
    MOD_TIME_HOUR_BIG,
    MOD_TIME_MIN_BIG,
    MOD_TIME_BIGCLOCK,
    MOD_TYPE_COUNT
} ModuleType;

/**
 * @brief A cell's exact footprint: height (rows) by width (columns), to match the HxW
 * naming people see. A module says which of these it is allowed to use, and each one
 * gets its own pixel tweaks.
 *
 * The order is the bit position the SZ_* flags use, so only ever add to the end.
 */
typedef enum
{
    MSIZE_1x2,  ///< Small and half width
    MSIZE_2x2,  ///< Big and half width
    MSIZE_1x4,  ///< Small and full width (the full width row's footprint)
    MSIZE_2x4,  ///< Big and full width
    MSIZE_COUNT
} ModuleSize;

// allowed-size bits for ModuleDef.sizes
#define SZ_1x2 (1 << MSIZE_1x2)
#define SZ_2x2 (1 << MSIZE_2x2)
#define SZ_1x4 (1 << MSIZE_1x4)
#define SZ_2x4 (1 << MSIZE_2x4)

typedef struct GridCtx GridCtx;

/**
 * @brief Which feature hub or hubs a module reads from, kept as a bitmask. A module
 * can read several (e.g. the composite datetime panel reads time and bluetooth). This writes down which
 * hubs a module cares about. Later on a per-hub repaint could check
 * `features & changed_feature`.
 */
typedef enum
{
    FEATURE_NONE    = 0,
    FEATURE_TIME    = 1 << 0,
    FEATURE_WEATHER = 1 << 1,
    FEATURE_HEALTH  = 1 << 2,
    FEATURE_SYSTEM  = 1 << 3,
    FEATURE_STOCK   = 1 << 4,
    FEATURE_CALENDAR = 1 << 5,
} ModuleFeature;

/**
 * @brief A module's colours for the VIBRANT theme. Any field left as GColorClear just
 * uses the mono base, so a module only needs to set the bits it wants coloured.
 */
typedef struct
{
    GColor accent;   ///< The header block, border, hatch, and progress colour
    GColor value;    ///< The big readout
    GColor icon;     ///< The recoloured icon
    GColor subtitle; ///< The caption line
} ModuleColors;

/**
 * @brief How the engine draws one module.
 *
 * A module gives a label for the header and a body that paints the area under it (e.g. the
 * battery gauge draws its own gauge). The engine wraps it in the shared frame.
 *
 * sizes is the bitmask of ModuleSize values the module is allowed to take. The layout
 * engine drops any cell whose module is not allowed at its size, so a broken LAYOUT
 * string can never leave a module stuck in a size it was not built for.
 *
 * A module can also be drawn with no header strip: the engine keeps the border, skips the
 * header and hands body the whole tile, so gctx->body is 14px taller and gctx->headerless is
 * set. A module only cares if its layout shifts (e.g. the analog clock swaps to its tall
 * plate); most just get the roomier body. headerless_sizes forces that for the sizes in the
 * mask (e.g. the clock bar's 1x4), and the per-module header toggle turns it on for any size.
 * Either way the engine still paints the frame, so every module goes through one drawing path.
 *
 * theme_alias lets a grouped panel borrow another module's colours: the daily forecast reads
 * as the hourly panel, the watchlist as the stock panel. 0 (MOD_EMPTY) means use its own.
 *
 * cleanup frees anything the module holds (e.g. a bitmap). NULL when it holds nothing, so the
 * catalog just skips it.
 */
typedef struct
{
    const char *label;                    ///< The header label
    const char *(*get_label)(ModuleSize size); ///< Header label worked out by size, or NULL to use label
    uint8_t     sizes;                    ///< Bitmask of ModuleSize values this module allows
    ModuleFeature features;               ///< Which hubs this module reads from, as a bitmask
    void (*body)(GridCtx *gctx);          ///< Paints the area under the header, or the whole tile when headerless
    uint8_t headerless_sizes;             ///< Bitmask of sizes always drawn with no header (0 means none)
    uint8_t theme_alias;                  ///< Borrow this module's colours (0 means use its own)
    void (*cleanup)(void);                ///< Free what the module holds (NULL means nothing to free)
} ModuleDef;

/**
 * @brief Whether a module is allowed to take a given size.
 *
 * @param type The module type.
 * @param size The cell size.
 * @return True when the module says it allows that size.
 */
bool module_allows_size(ModuleType type, ModuleSize size);

/**
 * @brief Looks up the description for a module type.
 *
 * @param type The module type.
 * @return The description (the MOD_EMPTY one for an unknown or empty type).
 */
const ModuleDef *module_def(ModuleType type);

/**
 * @brief Frees anything a module was holding onto (the weather icon picture).
 */
void modules_cleanup(void);

/** @} */
