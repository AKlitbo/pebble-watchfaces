/**
 * @file layout_places.c
 * @brief Walks a layout wire string record by record, reading only each record's panel.
 *
 * @ingroup mosaic_engine
 */
#include "layout_places.h"

#include "layout/layout_string.h"

bool layout_places_module(const char *layout, uint8_t type)
{
    for (const char *cursor = layout; cursor && *cursor; )
    {
        if (layout_parse_int(&cursor) == type)
        {
            return true;
        }

        // skip the rest of the record and its semicolon, onto the next record's panel
        while (*cursor && *cursor != ';')
        {
            cursor++;
        }
        if (*cursor == ';')
        {
            cursor++;
        }
    }

    return false;
}
