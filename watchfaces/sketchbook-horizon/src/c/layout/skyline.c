/**
 * @file skyline.c
 * @brief The Skyline scene's layout: its slots, where the clock sits for each layout mode, and
 * the hooks the shared layout engine runs. The common chrome, theme and slot work is in engine.c.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/scene.h"

#include "ui/fonts.h"
#include "ui/zone.h"
#include "draw/fonts.h"
#include "scene/skyline.h"
#include "theme/skyline.h"
#include "widgets/widgets.h"
#include "layout/chrome.h"
#include "layout/engine.h"
#include "settings_schema.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

// --- Text Slots ---
// the clock sits on the river, which starts above the digits whatever the lamps are doing, so
// there is always a strip of clear water between the reflections and the time
//
// a round screen narrows fastest at the bottom, so the clock is centred in the band between the
// water and the date rather than sat on either. the temperature comes off this stack entirely
#if defined(PBL_ROUND)
#define SLOT_TIME GRect(20, 143, 220, 100)    // clock on the water (Hand 82 here, not 72)
#define SLOT_TIME_SM GRect(20, 159, 220, 100) // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(20, 225, 220, 28)     // date under the clock (Hand 22)
#else
#define SLOT_TIME GRect(4, 108, 192, 86)      // clock on the water (Hand 72)
#define SLOT_TIME_SM GRect(4, 112, 192, 86)   // nudged down so the smaller tier keeps the baseline
#define SLOT_DATE GRect(4, 179, 192, 28)      // date under the clock (Hand 22)
#endif
// AM/PM sits beside the digits here, AM left and PM right, so the half of the day shows in its
// position as well as its letters. the bigger layouts have no room out there and put it above
// the colon instead
// the round clock is Hand 82, and at the widest time a marker beside it lands hard against the
// bezel, so there it takes the colon channel like the bigger layouts do
#define MERIDIEM_TOP PBL_IF_ROUND_ELSE(169, 131)  ///< 2px under the digits' cap line

// --- Date Top ---
// the date moves onto the strip but the readouts stay, so the clock only grows into the band the
// date left. same Hand 92 as the big clock, just lower and with less room under it
#if defined(PBL_ROUND)
#define SLOT_TIME_DATETOP GRect(20, 99, 220, 100)
#define SLOT_TIME_DATETOP_SM GRect(20, 115, 220, 100)
#else
#define SLOT_TIME_DATETOP GRect(0, 109, 200, 100)     // digits land at y 140..200, 4px clear of the readouts
#define SLOT_TIME_DATETOP_SM GRect(0, 125, 200, 100)
#endif
#define MERIDIEM_TOP_DATETOP PBL_IF_ROUND_ELSE(130, 140)  ///< The colon channel, on this layout's digits

// --- Big Clock ---
// the readouts go entirely and the clock takes everything they leave. same scene underneath
//
// the clock centres on the water, y=133 down, which is where the reflections stop
//
// full-screen width is the fit budget, not the position: the text centres either way, so the
// extra room only buys headroom against the tier dropping. the widest clock any format makes is
// 190px against 198, so the fallback never fires and the size never changes
#if defined(PBL_ROUND)
#define SLOT_TIME_BIG GRect(20, 117, 220, 100)   // the clock, filling the water (Hand 82)
#define SLOT_TIME_BIG_SM GRect(20, 133, 220, 100) // the tier this size never reaches
#else
#define SLOT_TIME_BIG GRect(0, 113, 200, 100)   // the clock, filling the water (Hand 92)
#define SLOT_TIME_BIG_SM GRect(0, 129, 200, 100) // the tier this size never reaches
#endif

// centred on the screen rather than on what is left of the strip, so it lines up with the clock.
// Hand 18 hangs 5px below its box and the strip is 20 tall, so the box starts above the screen to
// land the lettering on the strip's middle. it spans the clear run between the battery and the
// status glyphs, which is room enough for a long format
#if defined(PBL_ROUND)
#define SLOT_DATE_TOP GRect(45, 29, 170, 20)  // on the second strip, where the screen is 170 wide
#else
#define SLOT_DATE_TOP GRect(34, -2, 132, 20)
#endif
#define MERIDIEM_TOP_BIG PBL_IF_ROUND_ELSE(149, 145)  ///< The colon channel, on the big clock's digits

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
// the clock on the water with the date beneath it and the stats row along the bottom. the colours
// here are placeholders: the layout engine re-sets every one from the live palette before the
// first paint
static Zone s_zones[ZONE_COUNT] = {
    [ZONE_TIME]     = {.rect = SLOT_TIME,  .font_id = FONT_HAND_72, .align = GTextAlignmentCenter, .color = GColorBlack,
                       // the @beats token is wider than HH:MM and overflows Hand 72
                       .font_id_fallback = FONT_HAND_64, .rect_fallback = SLOT_TIME_SM},
    [ZONE_DATE]     = {.rect = SLOT_DATE,  .font_id = FONT_HAND_22, .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_18, .rect_fallback = SLOT_DATE},
    [ZONE_WEATHER]  = {.rect = SLOT_TEMP,  .font_id = FONT_HAND_18, .align = GTextAlignmentLeft, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_TEMP_SM},
    [ZONE_HR]       = {.rect = SLOT_HR,    .font_id = FONT_HAND_18, .align = GTextAlignmentCenter, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_HR_SM},
    [ZONE_STEPS]    = {.rect = SLOT_STEPS, .font_id = FONT_HAND_18, .align = GTextAlignmentRight, .color = GColorBlack,
                       .font_id_fallback = FONT_HAND_16, .rect_fallback = SLOT_STEPS_SM},
};

/**
 * @brief Point the time and date zones at whichever layout is in force.
 *
 * The three differ only in where these two sit and how big the clock is, so the zone table
 * carries the standard one and this moves them for the other two. Called before every rebuild,
 * so a settings change lands without duplicating the table.
 */
static void apply_layout(void)
{
    uint8_t layout = sketchbook_layout();

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
    // centred boxes that keep each glyph and its number together as a pair. a round screen has
    // neither reading left down here to place
#if !defined(PBL_ROUND)
    if (!sketchbook_has_hr())
    {
        s_zones[ZONE_WEATHER].rect = SLOT_TEMP_PAIR;
        s_zones[ZONE_WEATHER].rect_fallback = SLOT_TEMP_PAIR_SM;
        s_zones[ZONE_WEATHER].align = GTextAlignmentCenter;

        s_zones[ZONE_STEPS].rect = SLOT_STEPS_PAIR;
        s_zones[ZONE_STEPS].rect_fallback = SLOT_STEPS_PAIR_SM;
        s_zones[ZONE_STEPS].align = GTextAlignmentCenter;
    }
#endif

    // the clock is what actually differs: same Hand 92 for both roomier layouts, sitting higher
    // when the readouts are still under it
    if (layout == LAYOUT_BIG_CLOCK)
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME_BIG;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_BIG_SM;
        s_zones[ZONE_TIME].font_id = PBL_IF_ROUND_ELSE(FONT_HAND_82, FONT_HAND_92);
        s_zones[ZONE_TIME].font_id_fallback = FONT_HAND_72;
    }
    else if (layout == LAYOUT_DATE_TOP)
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME_DATETOP;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_DATETOP_SM;
        s_zones[ZONE_TIME].font_id = PBL_IF_ROUND_ELSE(FONT_HAND_82, FONT_HAND_92);
        s_zones[ZONE_TIME].font_id_fallback = FONT_HAND_72;
    }
    else
    {
        s_zones[ZONE_TIME].rect = SLOT_TIME;
        s_zones[ZONE_TIME].rect_fallback = SLOT_TIME_SM;
        // 82 on the circle rather than 92: at 92 the widest time runs 197px and the glass down
        // there is only 172 wide, so the outer digits ran into the bezel
        s_zones[ZONE_TIME].font_id = PBL_IF_ROUND_ELSE(FONT_HAND_82, FONT_HAND_72);
        s_zones[ZONE_TIME].font_id_fallback = PBL_IF_ROUND_ELSE(FONT_HAND_72, FONT_HAND_64);
    }
}

/**
 * @brief The cap line an above-the-colon marker sits on.
 *
 * Only the two Hand 92 layouts put it there, and their digits sit at different heights.
 */
static int meridiem_above_top(void)
{
    if (sketchbook_layout() == LAYOUT_BIG_CLOCK)
    {
        return MERIDIEM_TOP_BIG;
    }

    // the round standard layout draws Hand 82 too, at its own height, so it gets its own channel
    // rather than borrowing the date-top one
    return PBL_IF_ROUND_ELSE(
        sketchbook_layout() == LAYOUT_STANDARD ? MERIDIEM_TOP : MERIDIEM_TOP_DATETOP,
        MERIDIEM_TOP_DATETOP);
}

/** @brief The scene's palette for the hour, as the engine's opaque pointer. */
static const void *palette_for_theme(uint8_t theme, bool night)
{
    return skyline_palette_for_theme(theme, night);
}

/** @brief Paint the landscape, handing the scene back its own palette type. */
static void scene_draw(GContext *ctx, GRect bounds, const void *pal)
{
    skyline_scene_draw(ctx, bounds, pal);
}

static const SceneLayout s_layout = {
    .zones = s_zones,
    .palette_for_theme = palette_for_theme,
    .scene_draw = scene_draw,
    .apply_layout = apply_layout,
    .meridiem_above_top = meridiem_above_top,
    .meridiem_beside_top = MERIDIEM_TOP,
};

const SceneDef *skyline_scene_def(void)
{
    static SceneDef def = {.layout = &s_layout};
    def.geometry = skyline_geometry();
    return &def;
}

/** @} */
