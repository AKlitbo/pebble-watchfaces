/**
 * @file ops.h
 * @brief The catalog behind the two pickable ops slots. Each entry pairs an LCARS holder-box
 * word with a glyph and a value formatter, so picking a readout swaps all three together.
 *
 * The face draws its labels and glyphs at runtime rather than baking them into the frame, which
 * is what lets a slot change without new artwork for each theme.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief What a slot can show.
 *
 * The pick is persisted as its number, so entries are only ever appended. Reordering these
 * would repoint every watch in the field at a different readout.
 */
typedef enum
{
    OPS_HEART,
    OPS_STEPS,
    OPS_BATTERY,
    OPS_CALORIES,
    OPS_SLEEP,
    OPS_ACTIVE,
    OPS_MOON_PCT,
    OPS_MOON_PHASE,
    OPS_MOON_NEXT,
    OPS_SUNRISE,
    OPS_SUNSET,
    OPS_DAYLIGHT,
    OPS_SUN_NEXT,
    OPS_HUMIDITY,
    OPS_WIND,
    OPS_UV,
    OPS_HILO,
    OPS_JULIAN,
    OPS_DAY_OF_YEAR,
    OPS_WEEK,
    OPS_EMPTY,
    // this sits after the empty row rather than beside the other weather ones
    // the order here is the saved wire value so only the end of it is free
    OPS_WEATHER_TEMP,
    OPS_WEATHER_COND,
    OPS_SENSORS,
    OPS_EPOCH,
    OPS_BEATS,
    OPS_ZONE_1,
    OPS_COUNT
} OpsId;

/**
 * @brief One catalog entry: the holder-box word, the glyph, and the value formatter.
 *
 * The two _for hooks are for readouts whose word or glyph moves with the reading, like the sun
 * countdown flipping between DAWN IN and DUSK IN. An entry sets the plain field or the hook,
 * never both.
 */
typedef struct
{
    const char *label;                      ///< Fixed holder-box word, or NULL when label_for supplies it
    const char *(*label_for)(void);         ///< Word that moves with the reading, or NULL
    uint32_t    icon;                       ///< Fixed resource id, 0 for no glyph
    uint32_t  (*icon_for)(void);            ///< Glyph that moves with the reading, or NULL
    void      (*text)(char *out, size_t n); ///< Value formatter, or NULL to leave the slot blank
    bool        tall;                       ///< Fills both rows of its column, drawn by the face not the table
} OpsReadout;

/**
 * @brief The catalog entry for a stored pick.
 *
 * A number past the end of the catalog falls back to the empty entry, so a blob saved by a later
 * version can never point the face at nothing.
 *
 * @param id The stored OpsId.
 * @return The entry, never NULL.
 */
const OpsReadout *ops_entry(uint8_t id);

/**
 * @brief The holder-box word for an entry, resolved through label_for when it has one.
 *
 * @param entry The catalog entry.
 * @return The word, or "" when the slot draws no label.
 */
const char *ops_label(const OpsReadout *entry);

/**
 * @brief The glyph for an entry, resolved through icon_for when it has one.
 *
 * @param entry The catalog entry.
 * @return The resource id, or 0 when the slot draws no glyph.
 */
uint32_t ops_icon(const OpsReadout *entry);

/**
 * @brief Format an entry's value into a buffer, writing "" for a slot that shows nothing.
 *
 * @param entry The catalog entry.
 * @param out Output buffer.
 * @param n Buffer size.
 */
void ops_text(const OpsReadout *entry, char *out, size_t n);

/**
 * @brief Whether a pick fills both rows of its column rather than one.
 *
 * A tall pick is drawn by the face rather than off the catalog row, and only the top slot of a
 * column can hold one. The bottom slot is left undrawn while its column is tall.
 *
 * @param id The stored OpsId.
 * @return True for a tall entry.
 */
bool ops_is_tall(uint8_t id);

/** @} */
