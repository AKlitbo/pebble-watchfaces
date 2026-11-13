/**
 * @file dev.c
 * @brief The two hooks the dev plugin's listed walks call, which only a dev build compiles. A panel
 * walk shows each panel on its own, which is what the thumbnails tool crops the layout builder's
 * pictures from, and a layout walk shows each whole screen the config lists.
 *
 * @ingroup gridlock_dev
 */
#include "dev/dev.h"

#if DEV_MODE

#include <stdio.h>
#include <string.h>

#include "mosaic/engine/catalog.h"
#include "settings_schema.h"

void dev_show_panel(int panel, int size)
{
    // one block at the top left, two columns wide for the half-width sizes and four for the full
    // width ones, one row high for the short sizes and two for the tall ones. the rows are centred,
    // so a short panel sits at the same height whatever the size
    int width = size == MSIZE_1x4 || size == MSIZE_2x4 ? 4 : 2;
    int height = size == MSIZE_2x2 || size == MSIZE_2x4 ? 2 : 1;
    static char s_layout[24];

    snprintf(s_layout, sizeof(s_layout), "%d,0,0,%d,%d", panel, width, height);
    gridlock_set_layout(s_layout);

    // the saved custom colours would carry their chrome into the picture, so the walk shows each
    // panel plain, the way the layout builder's pictures do
    gridlock_set_custom_colors("0");
}

void dev_show_layout(const char *layout)
{
    // a layout can carry custom colours after a ~, in the same "~4" string the settings page sends,
    // and a layout without one shows plain. the layout itself never holds a ~
    static char s_layout[192];
    const char *colours = strchr(layout, '~');
    size_t length = colours ? (size_t)(colours - layout) : strlen(layout);

    if (length >= sizeof(s_layout))
    {
        length = sizeof(s_layout) - 1;
    }

    memcpy(s_layout, layout, length);
    s_layout[length] = '\0';

    gridlock_set_layout(s_layout);
    gridlock_set_custom_colors(colours ? colours : "0");
}

#endif
