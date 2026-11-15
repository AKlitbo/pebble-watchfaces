/**
 * @file dev_show.c
 * @brief The dev harness's layout hook, for the previews walk in paf.dev.config.json.
 *
 * Every step of that walk changes only settings, so the hook has no layout to show. The harness
 * links the hook for any step walk whether a step carries a layout or not, so it has to exist. A
 * release build leaves DEV_MODE off and carries none of it.
 *
 * @ingroup watchface-vscode
 */
#include <pebble.h>

#if DEV_MODE
#include "dev/dev_config.h"

void dev_show_layout(const char *layout)
{
    (void)layout;
}
#endif
