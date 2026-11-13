/**
 * @file layout_places.h
 * @brief Whether a layout wire string places a given panel, read straight off the string.
 *
 * A face holds its day, night, and Quiet Time layouts as wire strings, and only parses the one on
 * screen. Something that has to run whichever layout is showing, such as a reminder or a reading from
 * the phone, asks this about each string rather than parsing them all.
 *
 * @ingroup mosaic_engine
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Whether a layout string places a panel, without parsing it into blocks.
 *
 * Only the first field of each record is the panel, so a row, column, or size that happens to match
 * the panel's number does not count.
 *
 * @param layout The wire string ("module,row,col,w,h;..."), or NULL.
 * @param type The panel's ModuleType.
 * @return True when a record in the string names the panel.
 */
bool layout_places_module(const char *layout, uint8_t type);
