/**
 * @file icons.c
 * @brief Weather-icon resource lookup implementation
 *
 * The token-to-resource table lives in icons_table.g.h, generated from the shared
 * vocabulary in lib/js/weather/conditions.js (see lib/tools/build-conditions.js).
 */
#include "ui/weather/icons.h"

// Gated on HAS_WEATHER_ICONS, which waf_helpers defines when the face declares the weather
// icon set (see build_face). The generated tables reference RESOURCE_ID_ICON_WEATHER_NOW_*,
// so a face that does not draw weather icons (and so does not ship those resources) compiles
// this as an empty unit instead of failing on undeclared resource ids. Only per-face
// widgets.c call these, so a face that draws icons defines the flag by shipping the icons.
#if defined(HAS_WEATHER_ICONS)

#include "ui/weather/icons_table.g.h"
#include "ui/weather/icon_codes_table.g.h"

uint32_t wx_resource_for(const char *condition)
{
    return wx_resource_for_table(condition);
}

uint32_t wx_resource_for_forecast_code(uint8_t code)
{
    return wx_resource_for_code(code);
}

#endif  // HAS_WEATHER_ICONS
