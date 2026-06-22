/**
 * @file icons.c
 * @brief weather-icon resource lookup implementation
 */
#include "icons.h"

uint32_t wx_resource_for(const char *condition)
{
    if (!condition)                          return RESOURCE_ID_ICON_WI_NA;
    if (!strcmp(condition, "CLEAR"))         return RESOURCE_ID_ICON_WI_CLEAR;
    if (!strcmp(condition, "CLEAR_NIGHT"))   return RESOURCE_ID_ICON_WI_NIGHT_CLEAR;
    if (!strcmp(condition, "CLDY"))          return RESOURCE_ID_ICON_WI_CLOUDY;
    if (!strcmp(condition, "FOGGY"))         return RESOURCE_ID_ICON_WI_FOG;
    if (!strcmp(condition, "DRZL"))          return RESOURCE_ID_ICON_WI_DRIZZLE;
    if (!strcmp(condition, "FZDZ"))          return RESOURCE_ID_ICON_WI_SLEET;
    if (!strcmp(condition, "RAIN"))          return RESOURCE_ID_ICON_WI_RAIN;
    if (!strcmp(condition, "FZRN"))          return RESOURCE_ID_ICON_WI_SLEET;
    if (!strcmp(condition, "SNOW"))          return RESOURCE_ID_ICON_WI_SNOW;
    if (!strcmp(condition, "SHWR"))          return RESOURCE_ID_ICON_WI_SHOWERS;
    if (!strcmp(condition, "SNSH"))          return RESOURCE_ID_ICON_WI_SNOW_WIND;
    if (!strcmp(condition, "STRM"))          return RESOURCE_ID_ICON_WI_THUNDERSTORM;
    return RESOURCE_ID_ICON_WI_NA;
}
