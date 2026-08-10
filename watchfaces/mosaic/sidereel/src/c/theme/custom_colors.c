/**
 * @file custom_colors.c
 * @brief Decodes the packed appearance string into per-panel colours and flags.
 *
 * The format, which the encoder in core/pkjs/clay/builder/ts/theme/codec.ts has to match exactly:
 *
 *   "~3" <colour records> "|" <flag records>
 *
 * A colour record is five characters: the module id then its accent, value, icon and subtitle
 * channels. A flag record is three: the module id then the low and high halves of a packed flags
 * byte, where bit s is "hide header" at size s and bit 4+s is "hide border". Every character is
 * one symbol of the url-safe base64 alphabet, and "." means a channel was left alone.
 *
 * Only the "~3" shape carries records. The "0" default, an empty string, and anything the phone
 * has not re-sent in that shape all read as plain.
 *
 * @ingroup watchface-sidereel
 */
#include "custom_colors.h"

#include "persist_keys.h"

#include <string.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

// leads the string so it can be told from an empty or unrecognised one
#define CUSTOM_FORMAT_MARKER '~'

// the url-safe base64 alphabet, one symbol per 64-colour palette index. keep in step with the
// JS encoder: both map index 0..63 onto the same character
static const char s_alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/** @brief The colour half of the packed string, in its own persist slot. */
typedef struct
{
    uint8_t version;
    char    colors[SIDEREEL_CUSTOM_COLORS_LEN];
} SidereelCustomTheme;

/** @brief The flag half, in its own slot so neither can crowd the other past 256 bytes. */
typedef struct
{
    uint8_t version;
    char    flags[SIDEREEL_CUSTOM_FLAGS_LEN];
} SidereelCustomFlags;

static SidereelCustomTheme s_theme;
static SidereelCustomFlags s_flags_blob;

// the decoded tables, and whether they are up to date with the blobs
static SidereelCustomColor s_custom[MOD_TYPE_COUNT];
static uint8_t s_flags[MOD_TYPE_COUNT];
static bool s_parsed;

// these carry no message field: the combined string arrives on its own key and is split by
// sidereel_set_custom_colors, so the framework only persists the two blobs
const SettingsSchema sidereel_custom_flags_schema = {
    .key = SIDEREEL_CUSTOM_FLAGS_KEY,
    .version = 1,
    .min_versioned_size = sizeof(SidereelCustomFlags),
    .blob = &s_flags_blob,
    .blob_size = sizeof(s_flags_blob),
    .fields = NULL,
    .field_count = 0,
    .migrate = NULL,
    .companion = NULL,
};

const SettingsSchema sidereel_custom_theme_schema = {
    .key = SIDEREEL_CUSTOM_THEME_KEY,
    .version = 1,
    .min_versioned_size = sizeof(SidereelCustomTheme),
    .blob = &s_theme,
    .blob_size = sizeof(s_theme),
    .fields = NULL,
    .field_count = 0,
    .migrate = NULL,
    .companion = &sidereel_custom_flags_schema,
};

/**
 * @brief Turns one character into its palette index.
 *
 * @param ch The character.
 * @return 0 to 63, or -1 when it names no colour (the "." sentinel or any junk stays mono).
 */
static int color_index(char ch)
{
    if (ch == '\0')
    {
        return -1;
    }

    const char *at = strchr(s_alphabet, ch);

    return at ? (int)(at - s_alphabet) : -1;
}

/**
 * @brief Reads one channel character into a colour.
 *
 * @param ch The character.
 * @param out Receives the colour when one was named.
 * @return Whether the channel was set.
 */
static bool read_channel(char ch, GColor *out)
{
    int index = color_index(ch);

    if (index < 0)
    {
        return false;
    }

    // the palette runs 0..63 and an opaque GColor is 0xC0 plus that index
    out->argb = (uint8_t)(192 + index);

    return true;
}

/** @brief Spreads a packed flags byte, handed in as its two 6-bit halves, across a module. */
static void unpack_flags(uint8_t id, int low, int high)
{
    if (low < 0)
    {
        low = 0;
    }
    if (high < 0)
    {
        high = 0;
    }

    s_flags[id] = (uint8_t)((low & 63) | ((high & 63) << 6));
}

/** @brief strnlen, which the SDK does not carry. */
static size_t bounded_len(const char *blob, size_t cap)
{
    for (size_t i = 0; i < cap; i++)
    {
        if (blob[i] == '\0')
        {
            return i;
        }
    }

    return cap;
}

/** @brief Offset of a character within a bounded run, or the length when it is not there. */
static size_t bounded_find(const char *blob, size_t len, char ch)
{
    for (size_t i = 0; i < len; i++)
    {
        if (blob[i] == ch)
        {
            return i;
        }
    }

    return len;
}

/** @brief Decodes both blobs into the tables, unless that has already been done. */
static void ensure_parsed(void)
{
    if (s_parsed)
    {
        return;
    }

    s_parsed = true;

    memset(s_custom, 0, sizeof(s_custom));
    memset(s_flags, 0, sizeof(s_flags));

    const char *colors = s_theme.colors;
    size_t colors_len = bounded_len(colors, sizeof(s_theme.colors));

    if (colors_len < 2 || colors[0] != CUSTOM_FORMAT_MARKER || colors[1] != '3')
    {
        return;
    }

    const char *body = colors + 2;
    size_t body_len = colors_len - 2;

    const char *flags = s_flags_blob.flags;
    size_t flag_len = bounded_len(flags, sizeof(s_flags_blob.flags));

    // the flag section normally rides its own key, but a bar inside the colour blob ends the
    // colour section early and carries the flags after it, so a blob written as one joined
    // string still reads
    size_t bar = bounded_find(body, body_len, '|');
    if (bar < body_len)
    {
        flags = body + bar + 1;
        flag_len = body_len - bar - 1;
        body_len = bar;
    }

    for (size_t off = 0; off + 5 <= body_len; off += 5)
    {
        int id = color_index(body[off]);
        if (id <= 0 || id >= MOD_TYPE_COUNT)
        {
            continue;
        }

        SidereelCustomColor color = {0};
        color.accent_set = read_channel(body[off + 1], &color.accent);
        color.value_set = read_channel(body[off + 2], &color.value);
        color.icon_set = read_channel(body[off + 3], &color.icon);
        color.subtitle_set = read_channel(body[off + 4], &color.subtitle);
        s_custom[id] = color;
    }

    for (size_t off = 0; off + 3 <= flag_len; off += 3)
    {
        int id = color_index(flags[off]);
        if (id <= 0 || id >= MOD_TYPE_COUNT)
        {
            continue;
        }

        unpack_flags((uint8_t)id, color_index(flags[off + 1]), color_index(flags[off + 2]));
    }
}

SidereelCustomColor sidereel_custom_color(uint8_t module)
{
    ensure_parsed();

    if (module >= MOD_TYPE_COUNT)
    {
        return (SidereelCustomColor){0};
    }

    return s_custom[module];
}

bool sidereel_module_headerless(uint8_t module, ModuleSize size)
{
    ensure_parsed();

    if (module >= MOD_TYPE_COUNT || size >= MSIZE_COUNT)
    {
        return false;
    }

    return (s_flags[module] & (1u << size)) != 0;
}

bool sidereel_module_borderless(uint8_t module, ModuleSize size)
{
    ensure_parsed();

    if (module >= MOD_TYPE_COUNT || size >= MSIZE_COUNT)
    {
        return false;
    }

    return (s_flags[module] & (1u << (4 + size))) != 0;
}

void sidereel_set_custom_colors(const char *combined)
{
    if (!combined)
    {
        return;
    }

    size_t len = strlen(combined);
    size_t bar = bounded_find(combined, len, '|');

    // the colour section keeps the "~3" tag, and the flag section after the bar goes to its own
    // blob. a string with no bar is all colours
    size_t colors_len = bar < len ? bar : len;
    if (colors_len > sizeof(s_theme.colors) - 1)
    {
        colors_len = sizeof(s_theme.colors) - 1;
    }

    memcpy(s_theme.colors, combined, colors_len);
    s_theme.colors[colors_len] = '\0';

    s_flags_blob.flags[0] = '\0';
    if (bar < len)
    {
        size_t flags_len = len - bar - 1;
        if (flags_len > sizeof(s_flags_blob.flags) - 1)
        {
            flags_len = sizeof(s_flags_blob.flags) - 1;
        }

        memcpy(s_flags_blob.flags, combined + bar + 1, flags_len);
        s_flags_blob.flags[flags_len] = '\0';
    }

    s_parsed = false;  // the next read re-decodes
    settings_save();
}

void sidereel_get_custom_colors(char *out, size_t n)
{
    if (!out || n == 0)
    {
        return;
    }

    size_t colors_len = bounded_len(s_theme.colors, sizeof(s_theme.colors));
    size_t flags_len = bounded_len(s_flags_blob.flags, sizeof(s_flags_blob.flags));

    // nothing stored yet, so send the sentinel rather than an empty string: the framework drops
    // an empty cstring instead of storing it, so "" could never say "cleared"
    if (colors_len == 0)
    {
        strncpy(out, "0", n - 1);
        out[n - 1] = '\0';
        return;
    }

    // the two halves rejoin around the bar they were split on, each cut short of the buffer
    size_t at = colors_len < n - 1 ? colors_len : n - 1;
    memcpy(out, s_theme.colors, at);

    if (at < n - 1)
    {
        out[at++] = '|';
    }

    size_t room = n - 1 - at;
    size_t copy = flags_len < room ? flags_len : room;
    memcpy(out + at, s_flags_blob.flags, copy);
    out[at + copy] = '\0';
}

/** @} */
