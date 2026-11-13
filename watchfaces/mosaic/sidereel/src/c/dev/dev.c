/**
 * @file dev.c
 * @brief The hook the dev plugin's panel walk calls, which only a dev build compiles. It shows each
 * panel on its own in the top cell, which is where the thumbnails tool crops the layout builder's
 * pictures from.
 *
 * @ingroup watchface-sidereel
 */
#include "dev/dev.h"

#if DEV_MODE

#include <stdio.h>

#include "mosaic/engine/catalog.h"
#include "settings_schema.h"

void dev_show_panel(int panel, int size)
{
    // one block in the top cell above the pointer, one row high for a 1x2 and two for a 2x2. the
    // face reads no column or width, so both are written the way the layout builder writes them
    int height = size == MSIZE_2x2 ? 2 : 1;
    static char s_layout[24];

    snprintf(s_layout, sizeof(s_layout), "%d,0,0,2,%d", panel, height);
    sidereel_set_layout(s_layout);
}

#endif
