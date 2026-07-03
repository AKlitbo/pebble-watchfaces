/**
 * @file icons.c
 * @brief Weather-icon resource lookup implementation
 *
 * The token-to-resource table lives in icons_table.h, generated from the shared
 * vocabulary in lib/js/weather/conditions.js (see tools/generate-conditions.js).
 */
#include "ui/weather/icons.h"

#include "ui/weather/icons_table.h"

uint32_t wx_resource_for(const char *condition)
{
    return wx_resource_for_table(condition);
}
