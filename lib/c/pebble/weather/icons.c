/**
 * @file icons.c
 * @brief weather-icon resource lookup implementation
 *
 * The token-to-resource table lives in icons_table.h, generated from the shared
 * vocabulary in lib/js/weather/conditions.js (see tools/generate-conditions.js).
 */
#include "icons.h"

#include "icons_table.h"

uint32_t wx_resource_for(const char *condition)
{
    return wx_resource_for_table(condition);
}
