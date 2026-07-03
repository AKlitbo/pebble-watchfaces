/**
 * @file labels.c
 * @brief Condition-string to display-label lookup implementation
 *
 * The token-to-label tables live in labels_table.h, generated from the shared
 * vocabulary in lib/js/weather/conditions.js (see tools/generate-conditions.js).
 */
#include "ui/weather/labels.h"

#include "ui/weather/labels_table.h"

const char *wx_label_short(const char *condition)
{
    return wx_label_short_for_table(condition);
}

const char *wx_label_long(const char *condition)
{
    return wx_label_long_for_table(condition);
}
