/**
 * @file settings.c
 * @brief Persist API implementation
 */
#include "system/settings/settings.h"

#include "io/tuple_read.h"
#include "text/number_format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// head of the schema chain registered by settings_init (primary plus any companions)
static const SettingsSchema *s_primary;
// true when the primary key had no saved blob at init, i.e. storage was wiped (a fresh install
// or an update). lets the phone know to push its own config back instead of seeding from defaults
static bool s_was_fresh;
// known fields indexed by id for typed reads, drawn from every schema in the chain
static const SettingField *s_by_id[SETTING_COUNT];
// the schema that owns each indexed field, so a typed read hits the right blob
static const SettingsSchema *s_owner_by_id[SETTING_COUNT];

/**
 * @brief A pointer to a field's storage inside its owning schema's blob.
 *
 * @param schema The schema that owns the field.
 * @param field The setting field to get the pointer for.
 * @return A pointer to the field's storage.
 */
static void *field_ptr(const SettingsSchema *schema, const SettingField *field)
{
    return (uint8_t *)schema->blob + field->offset;
}

/**
 * @brief The version byte lives at the head of every schema's struct.
 *
 * @param schema The schema to read the version from.
 * @return The version byte from the schema's struct.
 */
static uint8_t get_version(const SettingsSchema *schema)
{
    return *(uint8_t *)schema->blob;
}

/**
 * @brief Set the version byte at the head of a schema's struct.
 *
 * @param schema The schema to write the version into.
 * @param version The version byte to set.
 */
static void set_version(const SettingsSchema *schema, uint8_t version)
{
    *(uint8_t *)schema->blob = version;
}

/**
 * @brief Index every schema's known fields by id, remembering the owning schema.
 *
 * So a shared read is a direct lookup. Face-only fields (id >= SETTING_COUNT)
 * are skipped. The face reads those off its own struct. The whole chain is
 * indexed, so a shared field may live in any schema (primary or companion) and
 * the owner table keeps the typed read pointed at the right blob.
 */
static void build_index(void)
{
    memset(s_by_id, 0, sizeof(s_by_id));
    memset(s_owner_by_id, 0, sizeof(s_owner_by_id));

    for (const SettingsSchema *schema = s_primary; schema; schema = schema->companion)
    {
        for (uint8_t i = 0; i < schema->field_count; i++)
        {
            const SettingField *field = &schema->fields[i];
            if (field->id < SETTING_COUNT)
            {
                s_by_id[field->id] = field;
                s_owner_by_id[field->id] = schema;
            }
        }
    }
}

/**
 * @brief Copy a cstring into a fixed buffer, NUL-terminating within size.
 *
 * A NULL src yields an empty string. Shared by the defaults, the sanitize
 * repair, and the inbound copy so the bounded-copy idiom lives in one place.
 *
 * @param dst The destination buffer.
 * @param src The source string, or NULL for empty.
 * @param size The size of the destination buffer.
 */
static void set_cstring(char *dst, const char *src, uint16_t size)
{
    if (src)
    {
        strncpy(dst, src, size - 1);
        dst[size - 1] = '\0';
    }
    else
    {
        dst[0] = '\0';
    }
}

/**
 * @brief Seed every field of one schema with its fresh-install default.
 *
 * The whole blob is zeroed first, so a schema that declares no fields (one the face fills in
 * itself rather than through the table) still resets to a known state instead of keeping
 * whatever a rejected blob left behind. Zero is the empty string for a cstring and 0 for a
 * number, which is what every field default then writes over.
 *
 * @param schema The schema to seed.
 */
static void apply_defaults(const SettingsSchema *schema)
{
    memset(schema->blob, 0, schema->blob_size);

    for (uint8_t i = 0; i < schema->field_count; i++)
    {
        const SettingField *field = &schema->fields[i];
        void *ptr = field_ptr(schema, field);

        switch (field->type)
        {
            case SETTING_BOOL:
                *(bool *)ptr = (field->default_num != 0);
                break;

            case SETTING_ENUM_U8:
                *(uint8_t *)ptr = (uint8_t)field->default_num;
                break;

            case SETTING_CSTRING:
                set_cstring((char *)ptr, field->default_str, field->size);
                break;
        }
    }
}

/**
 * @brief Clamp a damaged cstring back to its default.
 *
 * Valid means NUL-terminated within the buffer, non-empty, and free of control bytes. A place
 * name can arrive as UTF-8, so anything from 0x20 up counts as text and is left alone.
 *
 * @param schema The schema that owns the field.
 * @param field The setting field to sanitize.
 * @return true if the string was corrected, false otherwise.
 */
static bool sanitize_cstring(const SettingsSchema *schema, const SettingField *field)
{
    char *str = (char *)field_ptr(schema, field);

    if (cstring_is_clean(str, field->size))
    {
        return false;
    }

    set_cstring(str, field->default_str, field->size);
    return true;
}

/**
 * @brief Clamp every field of one schema to a legal value.
 *
 * Ensures a damaged blob never renders as garbage.
 *
 * @param schema The schema to sanitize.
 * @return true if anything was corrected so the caller can re-save.
 */
static bool sanitize(const SettingsSchema *schema)
{
    bool changed = false;

    for (uint8_t i = 0; i < schema->field_count; i++)
    {
        const SettingField *field = &schema->fields[i];
        uint8_t *byte = (uint8_t *)field_ptr(schema, field);

        switch (field->type)
        {
            case SETTING_BOOL:
                // a corrupt blob can leave a byte outside 0/1 so reset to the default
                if (*byte > 1)
                {
                    *byte = (uint8_t)(field->default_num != 0);
                    changed = true;
                }
                break;

            case SETTING_ENUM_U8:
                if (*byte >= field->enum_count)
                {
                    *byte = (uint8_t)field->default_num;
                    changed = true;
                }
                break;

            case SETTING_CSTRING:
                changed |= sanitize_cstring(schema, field);
                break;
        }
    }

    return changed;
}

/**
 * @brief Persist one schema's struct under its key.
 *
 * @param schema The schema to save.
 */
static void save_schema(const SettingsSchema *schema)
{
    persist_write_data(schema->key, schema->blob, schema->blob_size);
}

/**
 * @brief Load one schema from its key, applying defaults first and re-saving when needed.
 *
 * @param schema The schema to load.
 */
static void load_schema(const SettingsSchema *schema)
{
    apply_defaults(schema);
    set_version(schema, schema->version);

    if (!persist_exists(schema->key))
    {
        return;  // first run keeps the defaults
    }

    int stored = persist_get_size(schema->key);

    // let the face lift a legacy (pre-versioning) blob it recognises by size then
    // re-save versioned. a schema with no legacy history passes migrate = NULL
    if (schema->migrate && schema->migrate(stored))
    {
        sanitize(schema);
        set_version(schema, schema->version);
        save_schema(schema);
        return;
    }

    // versioned blob: at least this schema's frozen v1 size (smaller would shift
    // fields) and no bigger than the current struct. a short read upgrades an older
    // shorter version and the trailing fields keep their just-applied defaults
    if (stored >= schema->min_versioned_size && stored <= schema->blob_size)
    {
        persist_read_data(schema->key, schema->blob, schema->blob_size);

        uint8_t version = get_version(schema);
        if (version >= 1 && version <= schema->version)
        {
            // NOTE: non-append schema changes (reorder/remove/retype) need a per-version fixup here
            bool healed = sanitize(schema);

            // re-save if we upgraded an older version or had to repair a damaged field
            if (version < schema->version || healed)
            {
                set_version(schema, schema->version);
                save_schema(schema);
            }

            return;
        }
    }

    // unrecognized size or bad version: reset (the bad read may have clobbered the blob)
    apply_defaults(schema);
    set_version(schema, schema->version);
    save_schema(schema);
}

void settings_init(const SettingsSchema *head)
{
    s_primary = head;
    build_index();

    // a wipe clears every key, so the primary being absent means we booted with no saved settings
    s_was_fresh = !persist_exists(head->key);

    // load every schema in the chain (the primary plus any companions)
    for (const SettingsSchema *schema = head; schema; schema = schema->companion)
    {
        load_schema(schema);
    }
}

bool settings_was_fresh(void)
{
    return s_was_fresh;
}

void settings_save(void)
{
    for (const SettingsSchema *schema = s_primary; schema; schema = schema->companion)
    {
        save_schema(schema);
    }
}

uint8_t settings_u8(SettingId id)
{
    const SettingField *field = s_by_id[id];
    return field ? *(uint8_t *)field_ptr(s_owner_by_id[id], field) : 0;
}

const char *settings_str(SettingId id)
{
    const SettingField *field = s_by_id[id];
    return field ? (const char *)field_ptr(s_owner_by_id[id], field) : "";
}

void settings_set_u8(SettingId id, uint8_t value)
{
    const SettingField *field = s_by_id[id];
    if (field)
    {
        *(uint8_t *)field_ptr(s_owner_by_id[id], field) = value;
    }
}

uint8_t settings_enum_count(SettingId id)
{
    const SettingField *field = s_by_id[id];
    return field ? field->enum_count : 0;
}

void settings_serialize(DictionaryIterator *iter)
{
    for (const SettingsSchema *schema = s_primary; schema; schema = schema->companion)
    {
        for (uint8_t i = 0; i < schema->field_count; i++)
        {
            const SettingField *field = &schema->fields[i];
            uint8_t *byte = (uint8_t *)field_ptr(schema, field);

            DictionaryResult result = DICT_OK;

            switch (field->type)
            {
                case SETTING_BOOL:
                    result = dict_write_uint8(iter, *field->message_key, *byte ? 1 : 0);
                    break;

                case SETTING_ENUM_U8:
                {
                    char buf[4];  // enum value as a cstring (Clay reads selects as strings)
                    snprintf(buf, sizeof(buf), "%d", *byte);
                    result = dict_write_cstring(iter, *field->message_key, buf);
                    break;
                }

                case SETTING_CSTRING:
                    result = dict_write_cstring(iter, *field->message_key, (char *)byte);
                    break;
            }

            // the outbox is full: the rest of the seed would be dropped silently so stop
            // and leave the phone on its existing store rather than send a partial snapshot
            if (result != DICT_OK)
            {
                APP_LOG(APP_LOG_LEVEL_ERROR, "settings_serialize: outbox full at field %d", (int)i);
                return;
            }
        }
    }
}

SettingsInbound settings_apply_inbox(DictionaryIterator *iter)
{
    SettingsInbound result = {false, false, false};

    for (const SettingsSchema *schema = s_primary; schema; schema = schema->companion)
    {
        for (uint8_t i = 0; i < schema->field_count; i++)
        {
            const SettingField *field = &schema->fields[i];

            Tuple *tuple = dict_find(iter, *field->message_key);
            if (!tuple)
            {
                continue;
            }

            void *ptr = field_ptr(schema, field);

            switch (field->type)
            {
                case SETTING_BOOL:
                {
                    // -1 is a value the phone never sends for a bool so it doubles as the
                    // "wrong wire type" signal and the field keeps whatever it already held
                    int32_t on = tuple_int_or(tuple, -1);
                    if (on < 0)
                    {
                        continue;
                    }

                    *(bool *)ptr = (on == 1);
                    break;
                }

                case SETTING_ENUM_U8:  // arrives as a cstring so atoi it
                {
                    // atoi reads until it finds a terminator, so it has to be handed a string that
                    // has one inside itself. a raw tuple is only the phone's word for that
                    const char *digits = tuple_str_or(tuple, NULL);
                    if (!digits)
                    {
                        continue;
                    }

                    int value = atoi(digits);
                    if (value < 0 || value >= field->enum_count)
                    {
                        value = (int)field->default_num;  // out-of-range from the phone so clamp to default
                    }

                    *(uint8_t *)ptr = (uint8_t)value;
                    break;
                }

                case SETTING_CSTRING:
                {
                    const char *value = tuple_str_or(tuple, NULL);
                    if (!value || value[0] == '\0')
                    {
                        continue;  // nothing to store, so don't flag a change
                    }

                    set_cstring((char *)ptr, value, field->size);
                    break;
                }
            }

            result.changed = true;
            result.layout_changed |= field->affects_layout;
            result.weather_changed |= field->affects_weather;
        }
    }

    return result;
}
