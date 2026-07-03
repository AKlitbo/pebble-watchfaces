/**
 * @file settings.c
 * @brief Persist API implementation
 */
#include "system/settings/settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// head of the schema chain registered by settings_init (primary plus any companions)
static const SettingsSchema *s_primary;
// known fields indexed by id for typed reads. only ever the primary schema's fields
static const SettingField *s_by_id[SETTING_COUNT];

/**
 * @brief Whether a tuple carries an integer payload (TUPLE_INT or TUPLE_UINT).
 *
 * Inbox payloads are phone-controlled, so the wire type is checked before a
 * value is read through a type-specific accessor.
 *
 * @param tuple The tuple to check.
 * @return true if the tuple is an int or uint.
 */
static bool tuple_is_int(const Tuple *tuple)
{
    return tuple->type == TUPLE_INT || tuple->type == TUPLE_UINT;
}

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
 * @brief Index the primary schema's known fields by id.
 *
 * So a shared read is a direct lookup. Face-only fields (id >= SETTING_COUNT)
 * are skipped. The face reads those off its own struct. Only the primary schema
 * is indexed, so companion schemas must carry face-only fields.
 */
static void build_index(void)
{
    memset(s_by_id, 0, sizeof(s_by_id));

    for (uint8_t i = 0; i < s_primary->field_count; i++)
    {
        const SettingField *field = &s_primary->fields[i];
        if (field->id < SETTING_COUNT)
        {
            s_by_id[field->id] = field;
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
 * @param schema The schema to seed.
 */
static void apply_defaults(const SettingsSchema *schema)
{
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
 * Valid means NUL-terminated within the buffer, non-empty, and printable ASCII.
 *
 * @param schema The schema that owns the field.
 * @param field The setting field to sanitize.
 * @return true if the string was corrected, false otherwise.
 */
static bool sanitize_cstring(const SettingsSchema *schema, const SettingField *field)
{
    char *str = (char *)field_ptr(schema, field);

    bool ok = false;
    for (uint16_t i = 0; i < field->size; i++)
    {
        char c = str[i];
        if (c == '\0')
        {
            ok = (i > 0);
            break;
        }

        if (c < 0x20 || c > 0x7E)
        {
            break;
        }
    }

    if (ok)
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

    // load every schema in the chain (the primary plus any companions)
    for (const SettingsSchema *schema = head; schema; schema = schema->companion)
    {
        load_schema(schema);
    }
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
    return field ? *(uint8_t *)field_ptr(s_primary, field) : 0;
}

const char *settings_str(SettingId id)
{
    const SettingField *field = s_by_id[id];
    return field ? (const char *)field_ptr(s_primary, field) : "";
}

void settings_set_u8(SettingId id, uint8_t value)
{
    const SettingField *field = s_by_id[id];
    if (field)
    {
        *(uint8_t *)field_ptr(s_primary, field) = value;
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
                    if (!tuple_is_int(tuple))
                    {
                        continue;  // wrong wire type for a bool so skip rather than read a bogus pointer
                    }

                    *(bool *)ptr = (tuple->value->int32 == 1);
                    break;

                case SETTING_ENUM_U8:  // arrives as a cstring so atoi it
                {
                    if (tuple->type != TUPLE_CSTRING)
                    {
                        continue;
                    }

                    int value = atoi(tuple->value->cstring);
                    if (value < 0 || value >= field->enum_count)
                    {
                        value = (int)field->default_num;  // out-of-range from the phone so clamp to default
                    }

                    *(uint8_t *)ptr = (uint8_t)value;
                    break;
                }

                case SETTING_CSTRING:
                {
                    if (tuple->type != TUPLE_CSTRING)
                    {
                        continue;
                    }

                    // read through a char pointer so gcc does not flag the
                    // zero-length cstring[] flexible array member subscript
                    const char *value = tuple->value->cstring;
                    if (value[0] != '\0')
                    {
                        set_cstring((char *)ptr, value, field->size);
                    }

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
