/**
 * @file settings.h
 * @brief Persist API: a per-face SettingField table drives versioned load/save,
 * sanitize, the typed reads, and the appmessage round-trip, so shared code never
 * names or hardcodes a field
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Identity of a known (shared) setting, used for typed reads and indexing.
 *
 * Face-only settings sit in the same field table but carry an id of SETTING_COUNT
 * or greater, so they are serialised yet never indexed for a shared read.
 */
typedef enum
{
    SETTING_TEMPERATURE_UNIT,
    SETTING_DATE_FORMAT,
    SETTING_THEME,
    SETTING_STEPS_MODE,
    SETTING_DISTANCE_UNIT,
    SETTING_TIME_FORMAT,
    SETTING_BLUETOOTH_ICON,
    SETTING_BLUETOOTH_VIBE_CONNECT,
    SETTING_BLUETOOTH_VIBE_DISCONNECT,
    SETTING_BATTERY_DISPLAY,
    SETTING_COUNT
} SettingId;

/**
 * @brief How a setting encodes on the wire and how the sanitize pass clamps it.
 */
typedef enum
{
    SETTING_BOOL,     /**< wire uint8 0|1, default_num is the default (0 or 1) */
    SETTING_ENUM_U8,  /**< wire cstring of the number (Clay sends selects as strings) */
    SETTING_CSTRING   /**< wire cstring copied into a fixed buffer */
} SettingType;

/**
 * @brief Blueprint for a single setting.
 */
typedef struct
{
    SettingId   id;               /**< known id for typed reads, >= SETTING_COUNT means face-only */
    const uint32_t *message_key;  /**< &MESSAGE_KEY_* this field rides on (the SDK keys are runtime symbols) */
    SettingType type;             /**< drives both wire encoding and sanitize */
    uint16_t    offset;           /**< offsetof() into the owning face's struct */
    uint16_t    size;             /**< buffer size (SETTING_CSTRING only) */
    uint8_t     enum_count;       /**< clamp ceiling (SETTING_ENUM_U8 only) */
    uint32_t    default_num;      /**< fresh-install default for BOOL / ENUM_U8 */
    const char *default_str;      /**< fresh-install default for CSTRING */
    bool        affects_layout;   /**< a change re-renders the clock (date/time formats) */
    bool        affects_weather;  /**< a change re-requests weather (temperature unit) */
} SettingField;

/**
 * @brief A face's persistence identity.
 *
 * Its storage key, current schema version, frozen v1 blob size, the face struct to
 * load into (its first byte must be the uint8_t version), that struct's size, the
 * field table that drives defaults/sanitize/serialisation, and an optional migration
 * hook to rescue pre-versioned data.
 *
 * A face can persist more than one struct by chaining schemas through companion: each
 * schema points to the next and the last one is NULL. settings_init walks the chain so
 * load, save, serialise and inbox decode all cover every schema. Each link owns its own
 * key, version and blob, so a big optional block (e.g. a colour table) can sit in its own
 * key instead of bloating the main one. The typed reads (settings_u8/str) only ever look
 * at the first schema in the chain, so companion fields must be face-only (id >=
 * SETTING_COUNT).
 */
typedef struct SettingsSchema
{
    uint32_t key;                     /**< persist key for this face's blob */
    uint8_t version;                  /**< this face's current schema version */
    uint16_t min_versioned_size;      /**< smallest versioned blob accepted (frozen v1 size) */
    void *blob;                       /**< the face's settings struct (version byte first) */
    uint16_t blob_size;               /**< sizeof that struct */
    const SettingField *fields;       /**< the face's field table */
    uint8_t field_count;              /**< number of entries in fields */
    bool (*migrate)(int stored_size); /**< lift a legacy pre-versioning blob (true if handled). NULL when none */
    const struct SettingsSchema *companion; /**< next schema in the chain, or NULL */
} SettingsSchema;

/**
 * @brief Carries what an inbound settings message changed, so the caller can react.
 */
typedef struct
{
    bool changed;         /**< any field was updated (persist + repaint) */
    bool layout_changed;  /**< a date/time format changed (re-render the clock) */
    bool weather_changed; /**< the temperature unit changed (re-request weather) */
} SettingsInbound;

/**
 * @brief Load a face's settings from a trusted blob, applying defaults first.
 *
 * Walks the companion chain so every schema in it is loaded.
 *
 * @param schema The head schema defining the face's settings.
 */
void settings_init(const SettingsSchema *schema);

/**
 * @brief Persist the active face's structs under their keys (the whole chain).
 */
void settings_save(void);

/**
 * @brief Read a known uint8/bool setting by id.
 *
 * @param id The known setting to read.
 * @return The stored value, or 0 if the active face did not subscribe to it.
 */
uint8_t settings_u8(SettingId id);

/**
 * @brief Read a known string setting by id.
 *
 * @param id The known setting to read.
 * @return The stored string, or "" if the active face did not subscribe to it.
 */
const char *settings_str(SettingId id);

/**
 * @brief Write a known uint8/enum setting by id (in-memory only, does not persist).
 *
 * Mirror of settings_u8. A no-op if the active face didn't subscribe to the id. Used by the
 * dev theme walk to force a theme without touching persistence.
 *
 * @param id The known setting to write.
 * @param value The value to store.
 */
void settings_set_u8(SettingId id, uint8_t value);

/**
 * @brief How many values an enum setting allows (its sanitize ceiling).
 *
 * Lets a walk step through every option of an enum (e.g. every theme) without hand-counting.
 *
 * @param id The known enum setting.
 * @return The value count, or 0 if the active face didn't subscribe to it.
 */
uint8_t settings_enum_count(SettingId id);

/**
 * @brief Write every field of the active face into an outbox iterator.
 *
 * @param iter The dictionary iterator to encode into.
 */
void settings_serialize(DictionaryIterator *iter);

/**
 * @brief Apply any settings present in an inbox message to the active face.
 *
 * Decodes only. The caller persists and reacts based on the returned flags.
 *
 * @param iter The dictionary iterator to decode from.
 * @return Which categories of setting changed.
 */
SettingsInbound settings_apply_inbox(DictionaryIterator *iter);

/** @} */
