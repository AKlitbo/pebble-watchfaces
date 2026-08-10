/**
 * @file layout.h
 * @brief Spatial map for sidereel (200x228), and the face's setup/build/teardown API.
 *
 * The screen splits vertically: a black field on the left carrying two widget boxes and the hour
 * pennant, a white reel of minutes on the right, and a stippled band joining them. Everything
 * here is drawn, so these numbers are the only description of the layout there is.
 *
 * @ingroup watchface-sidereel
 */
#pragma once
#include <pebble.h>

#include "ui/engine/engine.h"

/**
 * @defgroup watchface-sidereel Sidereel Watchface
 * @brief The Sidereel Emery watchface: a scrolling minute reel read by an hour pointer.
 * @{
 */

// --- Screen ---
#define SCREEN_W 200
#define SCREEN_H 228
#define CENTRE_Y (SCREEN_H / 2)  // 114, the line the pennant tip and the centred minute share

// --- Day track ---
// a band hugging all four edges carries one whole day, so the perimeter is its axis. it is
// painted last of all, over the top of everything, the way a frame sits on a picture
#define BAND_THICK     5
#define BAND_PERIMETER (2 * (SCREEN_W + SCREEN_H))  // 856, so about 1.7 minutes to the pixel
#define BAND_MARK_HALF 4                            // half the width of the now marker

// --- Columns ---
// the track takes its 5px off the right first, then the reel, then the perforated strip, and the
// field gets what is left. 103 + 17 + 75 + 5 = 200
//
// the strip is 17 rather than 16 so its left edge lands two pixels off the panels rather than
// three, matching the gap they keep from the day track on the other side
#define REEL_W     75
#define SPROCKET_W 17
#define REEL_X     (SCREEN_W - BAND_THICK - REEL_W)  // 120
#define SPROCKET_X (REEL_X - SPROCKET_W)             // 103
#define FIELD_W    SPROCKET_X                        // 103

#define LEFT_FIELD     GRect(0, 0, FIELD_W, SCREEN_H)
#define SPROCKET_STRIP GRect(SPROCKET_X, 0, SPROCKET_W, SCREEN_H)
#define REEL_PANEL     GRect(REEL_X, 0, REEL_W, SCREEN_H)

// --- Reel rows ---
// at 56 the centred row spans 86..142, both neighbours sit fully on screen, and the next ones
// out show 30 of their 56 pixels, which is the half-cut look the reel wants
#define REEL_ROW_H      56
#define REEL_CENTRE_TOP (CENTRE_Y - REEL_ROW_H / 2)  // 86
#define REEL_TEXT_DY    (-9)                         // graphics_draw_text seats from the ascent, not the cap line

// how many rows either side of the centre get drawn. six rows covers both ends of the scroll
// offset, and the layer clips whatever hangs off
#define REEL_ROW_FIRST (-3)
#define REEL_ROW_LAST  2

// --- Highlight ---
// tighter than a row top and bottom, so it reads as a band hugging the number rather than as a
// second row. the pointer matches it exactly, so the two read as one bar across the face.
//
// it stops two pixels short of the reel's right edge so the panel colour runs on between it and
// the day track, the same two-pixel gutter the panels keep from the track on the other side
#define HIGHLIGHT_H   44
#define HIGHLIGHT_GAP 2
#define HIGHLIGHT     GRect(REEL_X, CENTRE_Y - HIGHLIGHT_H / 2, REEL_W - HIGHLIGHT_GAP, HIGHLIGHT_H)

// --- Pennant ---
// flush with the highlight band. the layer runs 127 wide so the tip, which reaches 5px past the
// panel edge and into the highlight, is not clipped
#define PENNANT_H     HIGHLIGHT_H
#define PENNANT_TOP   (CENTRE_Y - PENNANT_H / 2)  // 91
#define PENNANT_FRAME GRect(0, PENNANT_TOP, REEL_X + 7, PENNANT_H)

// where the solid nose starts, which is the far side of the screened band below
#define PENNANT_NOSE_X 46

// the hour is centred across the solid nose, from the end of the screened band to where the
// taper starts, so it sits square in the plain part of the pointer whatever the tail carries.
// the span starts a pixel in because the drawn nose does too, and it is lifted a couple of
// pixels so the hour reads level with the minute beside it
#define PENNANT_TEXT \
    GRect(PENNANT_NOSE_X + 1, -2, (CELL_X + CELL_W) - PENNANT_NOSE_X, PENNANT_H)

// the tail reads as a length of film leader: a perforation column punched through the trailing
// edge, then a screened band, with the nose left solid so the hour keeps its weight.
//
// the perforation origin is nudged so the holes land on the same rhythm as the sprocket strip
// beside the reel. sprockets_draw seats its first hole a fixed lead below the rect's top, and at
// PENNANT_TOP that lead puts this column's holes in step with the ones running down the screen
#define PENNANT_PERF GRect(CELL_X, 4, SPROCKET_W, PENNANT_H - 4)
#define PENNANT_TAIL \
    GRect(CELL_X + SPROCKET_W, 1, PENNANT_NOSE_X - (CELL_X + SPROCKET_W), PENNANT_H - 2)

// the two status glyphs stack down the pointer, on the screened band and clear of the
// perforations. the lower one sits further in because the pennant's bottom edge is where the
// taper starts to bite
#define PENNANT_BT    GRect(27, 4, 14, 14)
#define PENNANT_QUIET GRect(27, PENNANT_H - 16, 14, 14)

// --- Panel cells ---
// exactly gridlock's footprints, so a panel body copied from that face needs no retuning: its
// COL_W is 94, its ROW_UNIT is 40, and a tall block absorbs the 3px gap it spans, making a 2x2
// 83 tall
#define CELL_W    94
#define CELL_H    40
#define CELL_GAP  3
#define CELL_H_2X (CELL_H * 2 + CELL_GAP)  // 83
#define CELL_X    (BAND_THICK + 2)         // 7, clear of the day track with 2px to spare

// the pennant splits the field into two regions of 84px each. that is exactly a stacked pair of
// 1x2s (40 + 3 + 40) or a single 2x2, which is what lets a region be either
// gpath_draw_filled insets its top and bottom edge by a pixel, so the pointer paints 92..135
// rather than the 91..136 its rect claims. that is what makes this 2px clear of the panel above
// and the +2 below 2px clear of the one under it, measured off the render rather than the maths
#define REGION_TOP_Y    (BAND_THICK + 2)               // 7
// the bottom region clears the pennant by 2px, the same gap the top region keeps from the band.
// at 139 a stacked pair (or a 2x2) ends at 222, one clear pixel above the track at 223
#define REGION_BOTTOM_Y (PENNANT_TOP + PENNANT_H + 2)  // 139

// the grid the drag builder writes into is the whole watch, the same four by five gridlock uses.
// the reel owns columns 2 and 3 and the hour pointer owns row 2, so panels live in rows 0 and 1
// above it and rows 3 and 4 below
#define GRID_ROWS 5

/// The row the hour pointer runs across. Nothing can be placed on it
#define POINTER_ROW 2

/// The most panels that can be placed: two rows above the pointer and two below
#define CELL_COUNT 4

/// How long the LAYOUT wire string can get. Four records of "mm,r,c,w,h;" fits inside this
#define SIDEREEL_LAYOUT_LEN 64

/**
 * @brief The top of a grid row, in screen coordinates.
 *
 * Rows step by a cell plus its gap. Rows 3 and 4 start again from the bottom region rather than
 * carrying on down, which is what leaves the pointer its band in between.
 *
 * @param row The row index, 0 or 1 above the pointer and 3 or 4 below it.
 * @return The row's top edge.
 */
#define ROW_TOP(row)                                                          \
    ((row) < POINTER_ROW                                                      \
        ? REGION_TOP_Y + (row) * (CELL_H + CELL_GAP)                          \
        : REGION_BOTTOM_Y + ((row) - POINTER_ROW - 1) * (CELL_H + CELL_GAP))

// --- Repaint tags ---
// every slot must carry a non-zero tag: the engine repaints a tagless slot on every
// mark_dirty_tags call, which during a scroll means the whole face redraws each frame
#define TAG_TIME    (1u << 0)
#define TAG_WEATHER (1u << 1)
#define TAG_HEALTH  (1u << 2)
#define TAG_SYSTEM  (1u << 3)
#define TAG_REEL    (1u << 4)  // the scroll frames, and nothing else
#define TAG_CHROME  (1u << 5)  // static, so only a rebuild repaints it

/**
 * @brief Register fonts and pull in the palette.
 *
 * Call after the window exists and before engine_init.
 *
 * @param window The main window.
 */
void sidereel_setup(Window *window);

/**
 * @brief Fill the engine's slot list: chrome, reel, pennant, and the two widget boxes.
 *
 * @param out The slot array to fill.
 * @param max How many slots out can hold.
 * @param bounds The window's root bounds (for the full-window chrome slot).
 * @return How many slots were written.
 */
uint8_t sidereel_build(EngineSlot *out, uint8_t max, GRect bounds);

/**
 * @brief Re-pull the palette. Follow with engine_rebuild so every slot picks up the new colours.
 */
void sidereel_apply_theme(void);

/**
 * @brief Loads the user's chosen header font under FONT_HEADER, swapping out the previous one.
 *
 * Cheap to call on any settings push: it only reloads when the Header Font choice actually
 * changed. The teardown frees the final handle, so there is no separate cleanup.
 */
void sidereel_apply_header_font(void);

/**
 * @brief Tear down the fonts and every cached bitmap.
 */
void sidereel_teardown(void);

/** @} */
