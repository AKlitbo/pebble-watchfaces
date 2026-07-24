/**
 * @file number_format.c
 * @brief Pure number formatting
 */
#include "text/number_format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void number_group(char *buffer, size_t size, int value)
{
    if (size == 0)
    {
        return;
    }

    // strip the sign into an unsigned magnitude so INT_MIN stays well-defined
    // then walk the digits inserting separators
    char digits[16];
    unsigned int magnitude = (value < 0) ? -(unsigned int)value : (unsigned int)value;
    snprintf(digits, sizeof(digits), "%u", magnitude);

    size_t len = strlen(digits);
    char out[24];
    size_t w = 0;

    if (value < 0 && w < sizeof(out) - 1)
    {
        out[w++] = '-';
    }

    for (size_t i = 0; i < len && w < sizeof(out) - 1; i++)
    {
        // a separator precedes every group of three counted from the right
        if (i > 0 && (len - i) % 3 == 0)
        {
            out[w++] = '\'';
        }

        if (w < sizeof(out) - 1)
        {
            out[w++] = digits[i];
        }
    }

    out[w] = '\0';

    strncpy(buffer, out, size - 1);
    buffer[size - 1] = '\0';
}

void fmt_int_or_dash(char *buffer, size_t size, int value, const char *fmt)
{
    if (value < 0)
    {
        snprintf(buffer, size, "--");
    }
    else
    {
        snprintf(buffer, size, fmt, value);
    }
}

void fmt_hundredths(char *buffer, size_t size, int value)
{
    // pull the sign out front so a value between -1 and 0 still shows the minus, since the whole
    // part would be 0 and would otherwise drop it
    const char *sign = value < 0 ? "-" : "";
    int magnitude = abs(value);

    snprintf(buffer, size, "%s%d.%02d", sign, magnitude / 100, magnitude % 100);
}

void fmt_pct_signed(char *buffer, size_t size, int value)
{
    // same reason as above, plus a gain wears its plus so the direction reads without the colour
    const char *sign = value > 0 ? "+" : (value < 0 ? "-" : "");
    int magnitude = abs(value);

    snprintf(buffer, size, "%s%d.%02d%%", sign, magnitude / 100, magnitude % 100);
}

void copy_bounded(char *dst, uint8_t cap, const uint8_t *src, uint8_t len)
{
    if (cap == 0)
    {
        // nowhere to even put the terminator. checked up front because cap - 1 below would wrap
        // to 255 through the int promotion and copy a quarter kilobyte over whatever is there
        return;
    }

    uint8_t copy = len;
    if (copy > cap - 1)
    {
        copy = cap - 1;
    }
    for (uint8_t c = 0; c < copy; c++)
    {
        dst[c] = (char)src[c];
    }
    dst[copy] = '\0';
}

bool cstring_is_clean(const char *s, uint16_t size)
{
    if (!s)
    {
        return false;
    }

    for (uint16_t i = 0; i < size; i++)
    {
        // plain char is signed on the host and unsigned on the watch, so read it as unsigned
        // to make the range test mean the same thing in a spec and on the wrist
        unsigned char c = (unsigned char)s[i];
        if (c == '\0')
        {
            return i > 0;  // ends inside its room, and said something on the way
        }

        // control bytes are what a damaged blob looks like. everything from 0x20 up is kept,
        // including UTF-8 lead and continuation bytes, so a place name like Zurich spelled with
        // an umlaut keeps its bytes across a reboot
        if (c < 0x20 || c == 0x7F)
        {
            return false;
        }
    }

    return false;  // ran out of room without ever ending
}
