/**
 * @file settings.c
 * @brief persist API implementation
 */
#include "../settings/settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// face whose schema drove last settings_init
static const SettingsSchema *s_schema;
// known fields indexed by id, for typed reads
static const SettingField *s_by_id[SETTING_COUNT];

/**
 * @brief A pointer to a field's storage inside the active face's blob.
 *
 * @param field The setting field to get the pointer for.
 * @return A pointer to the field's storage.
 */
static void *field_ptr(const SettingField *field)
{
    return (uint8_t *)s_schema->blob + field->offset;
}

/**
 * @brief The version byte lives at the head of every face struct.
 *
 * @return The version byte from the active face's struct.
 */
static uint8_t get_version(void)
{
    return *(uint8_t *)s_schema->blob;
}

/**
 * @brief Set the version byte at the head of the face struct.
 *
 * @param version The version byte to set.
 */
static void set_version(uint8_t version)
{
    *(uint8_t *)s_schema->blob = version;
}

/**
 * @brief Index the known fields by id.
 *
 * So a shared read is a direct lookup. Face-only fields (id >= SETTING_COUNT)
 * are skipped - the face reads those off its own struct.
 */
static void build_index(void)
{
    memset(s_by_id, 0, sizeof(s_by_id));

    for (uint8_t i = 0; i < s_schema->field_count; i++)
    {
        const SettingField *field = &s_schema->fields[i];
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
 * @brief Seed every field with its fresh-install default.
 */
static void apply_defaults(void)
{
    for (uint8_t i = 0; i < s_schema->field_count; i++)
    {
        const SettingField *field = &s_schema->fields[i];
        void *ptr = field_ptr(field);

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
 * @param field The setting field to sanitize.
 * @return true if the string was corrected, false otherwise.
 */
static bool sanitize_cstring(const SettingField *field)
{
    char *str = (char *)field_ptr(field);

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
 * @brief Clamp every field to a legal value.
 *
 * Ensures a damaged blob never renders as garbage.
 *
 * @return true if anything was corrected so the caller can re-save.
 */
static bool sanitize(void)
{
    bool changed = false;

    for (uint8_t i = 0; i < s_schema->field_count; i++)
    {
        const SettingField *field = &s_schema->fields[i];
        uint8_t *byte = (uint8_t *)field_ptr(field);

        switch (field->type)
        {
            case SETTING_BOOL:
                // a corrupt blob can leave a byte outside 0/1, so reset to the default
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
                changed |= sanitize_cstring(field);
                break;
        }
    }

    return changed;
}

void settings_init(const SettingsSchema *schema)
{
    s_schema = schema;
    build_index();

    apply_defaults();
    set_version(schema->version);

    if (!persist_exists(schema->key))
    {
        return;  // first run keeps the defaults
    }

    int stored = persist_get_size(schema->key);

    // let the face lift a legacy (pre-versioning) blob it recognises by size, then
    // re-save versioned. a face with no legacy history passes migrate = NULL
    if (schema->migrate && schema->migrate(stored))
    {
        sanitize();
        set_version(schema->version);
        settings_save();
        return;
    }

    // versioned blob: at least this face's frozen v1 size (smaller would shift
    // fields), no bigger than the current struct. a short read upgrades an older,
    // shorter version, the trailing fields keeping their just-applied defaults
    if (stored >= schema->min_versioned_size && stored <= schema->blob_size)
    {
        persist_read_data(schema->key, schema->blob, schema->blob_size);

        uint8_t version = get_version();
        if (version >= 1 && version <= schema->version)
        {
            // NOTE: non-append schema changes (reorder/remove/retype) need a per-version fixup here
            bool healed = sanitize();

            // re-save if we upgraded an older version or had to repair a damaged field
            if (version < schema->version || healed)
            {
                set_version(schema->version);
                settings_save();
            }

            return;
        }
    }

    // unrecognized size or bad version: reset (the bad read may have clobbered the blob)
    apply_defaults();
    set_version(schema->version);
    settings_save();
}

void settings_save(void)
{
    persist_write_data(s_schema->key, s_schema->blob, s_schema->blob_size);
}

uint8_t settings_u8(SettingId id)
{
    const SettingField *field = s_by_id[id];
    return field ? *(uint8_t *)field_ptr(field) : 0;
}

const char *settings_str(SettingId id)
{
    const SettingField *field = s_by_id[id];
    return field ? (const char *)field_ptr(field) : "";
}

void settings_serialize(DictionaryIterator *iter)
{
    for (uint8_t i = 0; i < s_schema->field_count; i++)
    {
        const SettingField *field = &s_schema->fields[i];
        uint8_t *byte = (uint8_t *)field_ptr(field);

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

        // the outbox is full: the rest of the seed would be dropped silently, so stop
        // and leave the phone on its existing store rather than send a partial snapshot
        if (result != DICT_OK)
        {
            APP_LOG(APP_LOG_LEVEL_ERROR, "settings_serialize: outbox full at field %d", (int)i);
            return;
        }
    }
}

SettingsInbound settings_apply_inbox(DictionaryIterator *iter)
{
    SettingsInbound result = {false, false, false};

    for (uint8_t i = 0; i < s_schema->field_count; i++)
    {
        const SettingField *field = &s_schema->fields[i];

        Tuple *tuple = dict_find(iter, *field->message_key);
        if (!tuple)
        {
            continue;
        }

        void *ptr = field_ptr(field);

        switch (field->type)
        {
            case SETTING_BOOL:
                *(bool *)ptr = (tuple->value->int32 == 1);
                break;

            case SETTING_ENUM_U8:  // arrives as a cstring - atoi it
                *(uint8_t *)ptr = (uint8_t)atoi(tuple->value->cstring);
                break;

            case SETTING_CSTRING:
            {
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

    return result;
}
