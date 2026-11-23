/**
 * @file zones.c
 * @brief The slots every scene shares: the clock, the date, and the stats row, and where the clock
 * and date move for each layout mode.
 *
 * @ingroup watchface-sketchbook
 */
#include "layout/zones.h"

#include "draw/fonts.h"
#include "layout/chrome.h"
#include "layout/engine.h"
#include "settings_schema.h"
#include "widgets/widgets.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- Text Slots ---
// the clock sits on the scene's ground or water under the horizon, with the date beneath it
//
// a round screen narrows fastest at the bottom, so the clock is centred in the band between the
// scene and the date rather than sat on either. the temperature comes off this stack entirely and
// onto the scene
#if defined(PBL_ROUND)
#define SLOT_TIME GRect(20, 143, 220, 100)    // clock under the scene (Hand 82)
#define SLOT_TIME_SM GRect(20, 159, 220, 100) // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(20, 225, 220, 28)     // date under the clock (Hand 22)
#else
#define SLOT_TIME GRect(4, 108, 192, 86)      // clock under the scene (Hand 72)
#define SLOT_TIME_SM GRect(4, 112, 192, 86)   // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(4, 179, 192, 28)      // date under the clock (Hand 22)
#endif
// the bigger layouts have no room beside the digits and put AM/PM above the colon instead
// the round clock is Hand 82, and at the widest time a marker beside it lands hard against the
// bezel, so there it takes the colon channel like the bigger layouts do
#define MERIDIEM_TOP_ROUND 169  ///< The colon channel, on the round clock's digits

// --- Date Top ---
// the date moves onto the strip but the readouts stay, so the clock only grows into the band the
// date left. same Hand 92 as the big clock, just lower and with less room under it
//
// this and the big clock are the rectangle's alone, since a round screen only ships the standard
// layout
#if !defined(PBL_ROUND)
#define SLOT_TIME_DATETOP GRect(0, 109, 200, 100)     // digits land at y 140..200, 4px clear of the readouts
#define SLOT_TIME_DATETOP_SM GRect(0, 125, 200, 100)
#define MERIDIEM_TOP_DATETOP 140  ///< The colon channel, on this layout's digits

// --- Big Clock ---
// the readouts go entirely and the clock takes everything they leave. same scene underneath
//
// each scene picks the row its clock sits on, since how far down the ground starts is the scene's
// own. the smaller tier sits 16 lower to keep the baseline, and the colon channel is 32 down
//
// full-screen width is the fit budget, not the position: the text centres either way, so the
// extra room only buys headroom against the tier dropping. the widest clock any format makes is
// 190px against 198, so the fallback never fires and the size never changes
#define SLOT_TIME_BIG(y) GRect(0, (y), 200, 100)       // the clock, filling the ground (Hand 92)
#define SLOT_TIME_BIG_SM(y) GRect(0, (y) + 16, 200, 100) // the tier this size never reaches
#define MERIDIEM_TOP_BIG(y) ((y) + 32)  ///< The colon channel, on the big clock's digits

// centred on the screen rather than on what is left of the strip, so it lines up with the clock.
// Hand 18 hangs 5px below its box and the strip is 20 tall, so the box starts above the screen to
// land the lettering on the strip's middle. it spans the clear run between the battery and the
// status glyphs, which is room enough for a long format
#define SLOT_DATE_TOP GRect(34, -2, 132, 20)
#endif

// --- Temperature Slot ---
#define SLOT_TEMP GRect(19, 204, 47, 26)      // temperature, right of its thermometer (Hand 18)
#define SLOT_TEMP_SM GRect(21, 206, 47, 26)

// --- Stats Row, No Heart Rate ---
// an emery whose sensor cannot get a reading drops the row to two and re-centres rather than
// leaving a permanent "--" in the middle. both survivors take the treatment the heart rate had:
// text centred in its box with the glyph riding in front, so the pair stays centred whatever it
// reads
#if !defined(PBL_ROUND)
#define SLOT_TEMP_PAIR GRect(35, 204, 60, 26)
#define SLOT_STEPS_PAIR GRect(125, 204, 60, 26)
#define SLOT_TEMP_PAIR_SM GRect(37, 206, 60, 26)
#define SLOT_STEPS_PAIR_SM GRect(127, 206, 60, 26)
#endif

// --- Zone Table ---
// the clock under the scene with the date beneath it and the stats row along the bottom. the
// colours here are placeholders: the layout engine re-sets every one from the live palette before
// the first paint
//
// 82 on the circle rather than 92: at 92 the widest time runs 197px and the glass down there is
// only 172 wide, so the outer digits ran into the bezel
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME, .font_id = PBL_IF_ROUND_ELSE(FONT_HAND_82, FONT_HAND_72),
                       .align = GTextAlignmentCenter, .color = GColorBlack,
                       // the @beats token is wider than HH:MM and overflows the first tier
                       .font_id_fallback = PBL_IF_ROUND_ELSE(FONT_HAND_72, FONT_HAND_64),
                       .rect_fallback = SLOT_TIME_SM},
    [ZONE_DATE]     = {.rect = SLOT_DATE,  .font_id = FONT_HAND_22, .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_18, .rect_fallback = SLOT_DATE},
    [ZONE_WEATHER]  = {.rect = SLOT_TEMP,  .font_id = FONT_HAND_18, .align = GTextAlignmentLeft, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_TEMP_SM},
    [ZONE_HR]       = {.rect = SLOT_HR,    .font_id = FONT_HAND_18, .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_HR_SM},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS, .font_id = FONT_HAND_18, .align = GTextAlignmentRight, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_STEPS_SM},
};

// where the last layout_zones_apply put the colon channel, and whether it laid the row out with a
// heart rate. the slots, the readouts, and the glyphs all read these, so they never disagree
static int16_t s_meridiem_above_top;
#if !defined(PBL_ROUND)
static bool s_has_hr;
#endif

Zone *layout_zones(void)
{
    return s_zones;
}

void layout_zones_apply(int16_t clock_only_y)
{
#if defined(PBL_ROUND)
    (void)clock_only_y;
    s_meridiem_above_top = MERIDIEM_TOP_ROUND;
#else
    uint8_t layout = sketchbook_layout();
    s_has_hr = sketchbook_has_hr();

    // both date-up layouts share a date: on the strip, small, centred
    if (layout != LAYOUT_STANDARD)
    {
        s_zones[ZONE_DATE].rect = SLOT_DATE_TOP;
        s_zones[ZONE_DATE].font_id = FONT_HAND_18;
        s_zones[ZONE_DATE].rect_fallback = SLOT_DATE_TOP;
        s_zones[ZONE_DATE].font_id_fallback = FONT_HAND_16;
    }
    else
    {
        s_zones[ZONE_DATE].rect = SLOT_DATE;
        s_zones[ZONE_DATE].font_id = FONT_HAND_22;
        s_zones[ZONE_DATE].rect_fallback = SLOT_DATE;
        s_zones[ZONE_DATE].font_id_fallback = FONT_HAND_18;
    }

    // with no heart rate to show, the row is two readouts rather than three, so both move to the
    // centred boxes that keep each glyph and its number together as a pair
    // when the heart rate comes back they go back to the three-across boxes
    if (!s_has_hr)
    {
        s_zones[ZONE_WEATHER].rect = SLOT_TEMP_PAIR;
        s_zones[ZONE_WEATHER].rect_fallback = SLOT_TEMP_PAIR_SM;
        s_zones[ZONE_WEATHER].align = GTextAlignmentCenter;

        s_zones[ZONE_STEPS].rect = SLOT_STEPS_PAIR;
        s_zones[ZONE_STEPS].rect_fallback = SLOT_STEPS_PAIR_SM;
        s_zones[ZONE_STEPS].align = GTextAlignmentCenter;
    }
    else
    {
        s_zones[ZONE_WEATHER].rect = SLOT_TEMP;
        s_zones[ZONE_WEATHER].rect_fallback = SLOT_TEMP_SM;
        s_zones[ZONE_WEATHER].align = GTextAlignmentLeft;

        s_zones[ZONE_STEPS].rect = SLOT_STEPS;
        s_zones[ZONE_STEPS].rect_fallback = SLOT_STEPS_SM;
        s_zones[ZONE_STEPS].align = GTextAlignmentRight;
    }

    // the clock is what actually differs: same Hand 92 for both roomier layouts, sitting higher
    // when the readouts are still under it
    s_meridiem_above_top = MERIDIEM_TOP_DATETOP;
    if (layout == LAYOUT_BIG_CLOCK)
    {
        s_meridiem_above_top = MERIDIEM_TOP_BIG(clock_only_y);
        s_zones[ZONE_TIME].rect = SLOT_TIME_BIG(clock_only_y);
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_BIG_SM(clock_only_y);
        s_zones[ZONE_TIME].font_id = FONT_HAND_92;
        s_zones[ZONE_TIME].font_id_fallback = FONT_HAND_72;
    }
    else if (layout == LAYOUT_DATE_TOP)
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME_DATETOP;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_DATETOP_SM;
        s_zones[ZONE_TIME].font_id = FONT_HAND_92;
        s_zones[ZONE_TIME].font_id_fallback = FONT_HAND_72;
    }
    else
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_SM;
        s_zones[ZONE_TIME].font_id = FONT_HAND_72;
        s_zones[ZONE_TIME].font_id_fallback = FONT_HAND_64;
    }
#endif
}

int16_t layout_meridiem_above_top(void)
{
    return s_meridiem_above_top;
}

#if !defined(PBL_ROUND)
bool layout_zones_has_hr(void)
{
    return s_has_hr;
}
#endif

/** @} */
