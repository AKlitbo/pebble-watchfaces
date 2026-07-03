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

    // format the magnitude plainly then walk it inserting separators
    char digits[16];
    snprintf(digits, sizeof(digits), "%d", abs(value));

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
