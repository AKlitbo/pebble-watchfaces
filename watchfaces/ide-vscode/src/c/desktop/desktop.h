/**
 * @file desktop.h
 * @brief The desktops the face can wear, and the layout table each one draws from.
 *
 * Every desktop is a baked frame plus a table saying where the watch draws over it: the zone
 * rects, fonts, and colours, the weather glyph, the battery, and the bluetooth and Quiet Time
 * status. The frame pages under frame/ are drawn to the same pixels, so a rect here and the
 * chrome around it in the page move together.
 *
 * @ingroup watchface-vscode
 */
#pragma once
#include <pebble.h>

#include "ui/zone.h"
#include "draw/fonts.h"

/**
 * @addtogroup watchface-vscode
 * @{
 */

/** @brief The Desktop setting's values, in the order the settings page lists them. */
typedef enum
{
    DESKTOP_VSCODE,
    DESKTOP_WIN95,
    DESKTOP_WIN2000,
    DESKTOP_DOS,
    DESKTOP_DOS_IDE,
    DESKTOP_COUNT
} DesktopId;

/** @brief The engine text-slot ids, one per live readout. */
enum
{
    ZONE_TIME,
    ZONE_DATE,
    ZONE_COND,
    ZONE_WEATHER,
    ZONE_HR,
    ZONE_STEPS,
    ZONE_COUNT
};

/** @brief Writes a readout's text, the same shape as the framework's readouts. */
typedef void (*ReadoutText)(char *out, size_t n);

/** @brief The space between the packed status items, measured between their visible art. */
#define STATUS_GAP 4

/** @brief How a desktop draws the battery beside its percentage. */
typedef enum
{
    BATT_STYLE_OUTLINE,  ///< an outlined cell that fills to the charge level
    BATT_STYLE_GAUGE,    ///< a row of ten blocks, shaded past the charge level
} BattStyle;

/**
 * @brief One desktop's layout.
 *
 * A zero-sized rect leaves that piece out. VS Code takes its colours from the Theme setting, so
 * its table holds the Dark+ colours and layout.c paints the chosen theme's over them.
 *
 * A desktop that shows its status as glyphs packs them leftwards from the battery icon: the icon,
 * the percentage, bluetooth, then Quiet Time, STATUS_GAP apart, with anything hidden closing up.
 * Only the icon's x is fixed there, and the percentage, bluetooth, and Quiet Time rects give just
 * their row and size. A desktop that writes its status as words gives each a fixed rect.
 */
typedef struct DesktopLayout
{
    uint32_t    background;            ///< the baked frame, or 0 when the theme picks it
    uint32_t    fonts[FONT_COUNT];     ///< the font resource for each slot, 0 for a slot left empty
    const char *system_fonts[FONT_COUNT]; ///< a built-in font key for a slot, which costs no resource
    Zone        zones[ZONE_COUNT];     ///< where each readout sits, in which font and colour
    ReadoutText text[ZONE_COUNT];      ///< a readout written its own way, or NULL for the plain one
    uint8_t     clock_pitch;           ///< draw the clock a glyph at a time, each digit this far apart, or 0 for plain text
    uint8_t     clock_colon_pitch;     ///< the colon's share of the width when the clock is drawn a glyph at a time
    GRect       wx_icon;               ///< the weather glyph
    GColor      wx_color;              ///< the weather glyph's tint
    BattStyle   batt_style;            ///< the cell or the gauge
    GRect       batt_icon;             ///< the cell or the gauge, hidden by Battery Display
    GRect       batt_text;             ///< the percentage
    GTextAlignment batt_align;         ///< how the percentage sits in a fixed rect
    bool        batt_text_always;      ///< the percentage has a labelled spot of its own, so it shows whatever Battery Display says and stays out of the packed row
    FontId      batt_font;             ///< the percentage's font slot, also used for status words
    GColor      batt_color;            ///< the percentage, and the icon's outline or full blocks
    GColor      batt_fill;             ///< the cell's fill above 20%
    GRect       bt;                    ///< the bluetooth glyph, or its LINK or DOWN text
    GRect       quiet;                 ///< the Quiet Time glyph, or its ON or OFF text
    bool        status_text;           ///< bluetooth and Quiet Time as words rather than glyphs
    GColor      status_color;          ///< the glyphs' tint, or the text while on
    GColor      status_off_color;      ///< the text while off
    GColor      quiet_color;           ///< Quiet Time's ON text
} DesktopLayout;

/**
 * @brief The layout table for a desktop.
 *
 * @param desktop The Desktop setting value. Anything out of range gets VS Code.
 * @return The table, which lives for the whole run.
 */
const DesktopLayout *desktop_layout(uint8_t desktop);

/** @} */
