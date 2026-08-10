/**
 * @file layout_string.c
 * @brief Parses the LAYOUT wire string into the block cache.
 *
 * The cache is rebuilt only when the string changes, which is cheaper than holding a copy to
 * diff against and cheaper than re-parsing on every repaint.
 *
 * @ingroup watchface-sidereel
 */
#include "layout_string.h"

#include "mosaic/engine/catalog.h"
#include "settings_schema.h"

#include <string.h>

/**
 * @addtogroup watchface-sidereel
 * @{
 */

static SidereelBlock s_blocks[CELL_COUNT];
static uint8_t s_count;
static char s_parsed[SIDEREEL_LAYOUT_LEN];  // the string the cache was built from

/**
 * @brief Reads a run of digits and moves the cursor past them.
 *
 * @param cursor The cursor, moved past the digits.
 * @return The value read, 0 when there are no digits.
 */
static int parse_int(const char **cursor)
{
    int value = 0;

    while (**cursor >= '0' && **cursor <= '9')
    {
        value = value * 10 + (**cursor - '0');
        (*cursor)++;
    }

    return value;
}

/** @brief Steps the cursor past the rest of the current record. */
static void skip_record(const char **cursor)
{
    while (**cursor && **cursor != ';')
    {
        (*cursor)++;
    }

    if (**cursor == ';')
    {
        (*cursor)++;
    }
}

/** @brief Whether a block covering rows [row, row + height) would touch the pointer row. */
static bool spans_pointer(int row, int height)
{
    return row <= POINTER_ROW && row + height > POINTER_ROW;
}

/**
 * @brief Whether a block overlaps one already placed.
 *
 * The builder will not write an overlapping layout, but an imported or corrupt string can, and
 * two panels drawn over each other is worse than one silently dropped.
 */
static bool overlaps(int row, int height)
{
    for (uint8_t i = 0; i < s_count; i++)
    {
        int top = s_blocks[i].row;
        int bottom = top + s_blocks[i].height;

        if (row < bottom && row + height > top)
        {
            return true;
        }
    }

    return false;
}

/** @brief Rebuilds the block cache, unless it already matches the stored string. */
static void ensure_parsed(void)
{
    const char *layout = sidereel_layout();

    if (strncmp(s_parsed, layout, sizeof(s_parsed)) == 0)
    {
        return;
    }

    strncpy(s_parsed, layout, sizeof(s_parsed) - 1);
    s_parsed[sizeof(s_parsed) - 1] = '\0';
    s_count = 0;

    for (const char *cursor = layout; cursor && *cursor && s_count < CELL_COUNT; )
    {
        int module = parse_int(&cursor);

        // every field is read in order, and the two this grid cannot vary are dropped on the
        // floor: there is one column and one width
        if (*cursor == ',')
        {
            cursor++;
        }
        int row = parse_int(&cursor);

        if (*cursor == ',')
        {
            cursor++;
        }
        (void)parse_int(&cursor);  // col

        if (*cursor == ',')
        {
            cursor++;
        }
        (void)parse_int(&cursor);  // w

        if (*cursor == ',')
        {
            cursor++;
        }
        int height = parse_int(&cursor) >= 2 ? 2 : 1;

        skip_record(&cursor);

        // module 0 is the empty sentinel, and an id past the catalog is a layout written by a
        // face with more panels than this one
        if (module <= 0 || module >= MOD_TYPE_COUNT)
        {
            continue;
        }

        if (row < 0)
        {
            row = 0;
        }
        if (row + height > GRID_ROWS)
        {
            row = GRID_ROWS - height;
        }
        if (spans_pointer(row, height) || overlaps(row, height))
        {
            continue;
        }

        // a panel that does not come in the size it was placed at would draw nothing, so drop it
        // here instead and leave the cell honestly empty
        if (!module_allows_size((ModuleType)module, height >= 2 ? MSIZE_2x2 : MSIZE_1x2))
        {
            continue;
        }

        s_blocks[s_count].module = (uint8_t)module;
        s_blocks[s_count].row = (uint8_t)row;
        s_blocks[s_count].height = (uint8_t)height;
        s_count++;
    }
}

uint8_t sidereel_block_count(void)
{
    ensure_parsed();

    return s_count;
}

SidereelBlock sidereel_block(uint8_t index)
{
    ensure_parsed();

    if (index >= s_count)
    {
        return (SidereelBlock){0, 0, 1};
    }

    return s_blocks[index];
}

/** @} */
