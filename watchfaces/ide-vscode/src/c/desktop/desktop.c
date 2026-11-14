/**
 * @file desktop.c
 * @brief The layout table for each desktop.
 *
 * Every rect is in watch pixels and matches the frame page it draws over. A font of size N puts
 * its baseline N pixels below the top of the rect, so each text rect starts where the font's own
 * blank space above the capitals lines the ink up with the chrome. Share Tech Mono 14 inks from
 * y+4, VT323 20 from y+9, VT323 75 from y+33, VT323 84 from y+36, VT323 100 from y+44, and
 * Teko 82 from y+30.
 *
 * @ingroup watchface-vscode
 */
#include "desktop.h"

#include "ui/readouts.h"

/**
 * @addtogroup watchface-vscode
 * @{
 */

/** The date as a line of code, quoted and ended with a semicolon. */
static void date_quoted(char *out, size_t n)
{
    char date[24];
    readout_date(date, sizeof(date));
    snprintf(out, n, "\"%s\";", date);
}

/** The sky as a string value in a debugger's watch list. */
static void cond_quoted(char *out, size_t n)
{
    char cond[16];
    readout_weather_cond(cond, sizeof(cond));
    snprintf(out, n, "\"%s\"", cond);
}

/** The steps with the space before a distance unit dropped, so 12.4 KM fits a narrow column as 12.4KM. */
static void steps_compact(char *out, size_t n)
{
    char steps[16];
    readout_steps(steps, sizeof(steps));

    size_t at = 0;
    for (const char *from = steps; *from && at + 1 < n; from++)
    {
        if (*from != ' ')
        {
            out[at++] = *from;
        }
    }

    out[at] = '\0';
}

/** The heart rate with its unit, or the plain dashes when there is no reading. */
static void hr_bpm(char *out, size_t n)
{
    char hr[8];
    readout_hr(hr, sizeof(hr));

    if (hr[0] == '-')
    {
        snprintf(out, n, "%s", hr);
        return;
    }

    snprintf(out, n, "%s bpm", hr);
}

static const DesktopLayout s_desktops[DESKTOP_COUNT] = {
    // the editor right of an 18px activity bar, with a terminal panel at the foot. the colours here
    // are Dark+, and the theme paints its own over them
    [DESKTOP_VSCODE] = {
        .fonts = {
            [FONT_CLOCK] = RESOURCE_ID_FONT_TEKO_82,
            [FONT_CLOCK_SMALL] = RESOURCE_ID_FONT_TEKO_78,
            [FONT_DATE] = RESOURCE_ID_FONT_STM_20,
            [FONT_TEXT] = RESOURCE_ID_FONT_STM_14,
            [FONT_SMALL] = RESOURCE_ID_FONT_STM_12,
        },
        .zones = {
            // the widest time in Teko 82, 00:04, is 166px of ink, which leaves it 9px and 7px inside
            // the editor. the rect is 172 wide so the 179px .beats reading overflows it and drops
            // to Teko 78, and both sizes end at y104
            [ZONE_TIME] = {.rect = {{23, 22}, {172, 86}}, .font_id = FONT_CLOCK, .align = GTextAlignmentCenter,
                           .color = GColorVividCerulean,
                           .font_id_fallback = FONT_CLOCK_SMALL, .rect_fallback = {{23, 26}, {172, 82}}},
            [ZONE_DATE] = {.rect = {{18, 110}, {182, 22}}, .font_id = FONT_DATE, .align = GTextAlignmentCenter,
                           .color = GColorRajah,
                           .font_id_fallback = FONT_TEXT, .rect_fallback = {{18, 110}, {182, 22}}},
            // the terminal's 2x2 grid. weather condition over temp, heart rate over steps
            [ZONE_COND] = {.rect = {{38, 176}, {70, 18}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                           .color = GColorMediumAquamarine,
                           .font_id_fallback = FONT_SMALL, .rect_fallback = {{38, 176}, {70, 18}}},
            [ZONE_WEATHER] = {.rect = {{38, 189}, {70, 18}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                              .color = GColorMediumAquamarine},
            [ZONE_HR] = {.rect = {{124, 176}, {76, 18}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                         .color = GColorRed},
            [ZONE_STEPS] = {.rect = {{124, 189}, {76, 18}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                            .color = GColorCeleste},
        },
        // in the 12px column above the baked thermometer
        .wx_icon = {{24, 179}, {12, 12}},
        .wx_color = GColorMediumAquamarine,
        // the right end of the tab strip, packed from the cell at the right edge
        .batt_style = BATT_STYLE_OUTLINE,
        .batt_icon = {{181, 24}, {13, 8}},
        .batt_text = {{0, 19}, {0, 18}},
        .batt_font = FONT_TEXT,
        .batt_color = GColorMediumAquamarine,
        .batt_fill = GColorMediumAquamarine,
        .bt = {{0, 22}, {10, 10}},
        .quiet = {{0, 24}, {11, 8}},
        .status_color = GColorMediumAquamarine,
    },

    // a code editor window over a prompt window, on a teal desktop with a taskbar
    [DESKTOP_WIN95] = {
        .background = RESOURCE_ID_IMAGE_BG_WIN95,
        .fonts = {
            [FONT_CLOCK] = RESOURCE_ID_FONT_VT_84,
            [FONT_DATE] = RESOURCE_ID_FONT_STM_14,
            [FONT_TEXT] = RESOURCE_ID_FONT_VT_20,
            [FONT_SMALL] = RESOURCE_ID_FONT_STM_12,
        },
        .zones = {
            // the clock and date sit centred in the text area right of the gutter (x21..193), under
            // the baked comment, inked y53..101 and y110..120. VT323 84 lands cleanly on the watch's
            // pixels where 80 does not, and its widest time is 170px, so it fills the text area.
            // the .beats reading fits it too, so the clock has no smaller size
            [ZONE_TIME] = {.rect = {{21, 17}, {172, 90}}, .font_id = FONT_CLOCK, .align = GTextAlignmentCenter,
                           .color = GColorBlack},
            [ZONE_DATE] = {.rect = {{21, 106}, {172, 18}}, .font_id = FONT_DATE, .align = GTextAlignmentCenter,
                           .color = GColorDukeBlue,
                           .font_id_fallback = FONT_SMALL, .rect_fallback = {{21, 108}, {172, 16}}},
            // the prompt's two rows under the baked status.bat line, each value after its baked label
            [ZONE_COND] = {.rect = {{104, 170}, {90, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                           .color = GColorWhite},
            [ZONE_WEATHER] = {.rect = {{42, 170}, {58, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                              .color = GColorWhite},
            [ZONE_HR] = {.rect = {{42, 184}, {58, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                         .color = GColorWhite},
            [ZONE_STEPS] = {.rect = {{136, 184}, {58, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                            .color = GColorWhite},
        },
        .text = {
            [ZONE_DATE] = date_quoted,
        },
        // the taskbar tray (x110..198, y212..226), packed from the cell at its right end. the cell,
        // bluetooth, and Quiet Time art all centre on y218.5, a pixel clear of the tray's bevels
        .batt_style = BATT_STYLE_OUTLINE,
        .batt_icon = {{181, 215}, {13, 8}},
        .batt_text = {{0, 205}, {0, 24}},
        .batt_font = FONT_TEXT,
        .batt_color = GColorBlack,
        .batt_fill = GColorIslamicGreen,
        .bt = {{0, 214}, {10, 10}},
        .quiet = {{0, 215}, {11, 8}},
        .status_color = GColorBlack,
    },

    // a Watch Properties dialog on a slate desktop, its Time tab holding a Current Time group over a
    // Sensors group of four labelled fields, the weather pair stacked on the left and the health pair
    // on the right
    [DESKTOP_WIN2000] = {
        .background = RESOURCE_ID_IMAGE_BG_WIN2000,
        .fonts = {
            [FONT_CLOCK] = RESOURCE_ID_FONT_VT_75,
        },
        // the firmware's Gothic Bold is hand-hinted for the watch, so it reads crisper than any
        // sans the font tool can rasterize at this size. it inks its capitals y+7..y+18
        .system_fonts = {
            [FONT_DATE] = FONT_KEY_GOTHIC_18_BOLD,
            [FONT_TEXT] = FONT_KEY_GOTHIC_18_BOLD,
        },
        .zones = {
            // the clock and date sit centred in the Current Time group (x10..190), inked y50..92
            // and y99..109, so the group's label gets 4px of air above the clock
            [ZONE_TIME] = {.rect = {{11, 17}, {178, 80}}, .font_id = FONT_CLOCK, .align = GTextAlignmentCenter,
                           .color = GColorBlack},
            [ZONE_DATE] = {.rect = {{11, 92}, {178, 22}}, .font_id = FONT_DATE, .align = GTextAlignmentCenter,
                           .color = GColorOxfordBlue},
            // each value's capitals sit centred in its sunken field, 2px clear of the bevel each side
            [ZONE_WEATHER] = {.rect = {{19, 138}, {75, 22}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                              .color = GColorBlack},
            [ZONE_COND] = {.rect = {{19, 171}, {75, 22}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                           .color = GColorBlack},
            [ZONE_HR] = {.rect = {{107, 138}, {75, 22}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                         .color = GColorBlack},
            [ZONE_STEPS] = {.rect = {{107, 171}, {75, 22}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                            .color = GColorBlack},
        },
        .text = {
            [ZONE_HR] = hr_bpm,
        },
        // the taskbar tray (x110..198, y212..226), packed from the cell at its right end. the cell,
        // bluetooth, Quiet Time, and the percentage's capitals all centre on about y219
        .batt_style = BATT_STYLE_OUTLINE,
        .batt_icon = {{181, 215}, {13, 8}},
        .batt_text = {{0, 207}, {0, 22}},
        .batt_font = FONT_TEXT,
        .batt_color = GColorBlack,
        .batt_fill = GColorIslamicGreen,
        .bt = {{0, 214}, {10, 10}},
        .quiet = {{0, 215}, {11, 8}},
        .status_color = GColorBlack,
    },

    // a two-panel file manager: a clock panel across the top, then a file list beside an Info
    // panel of labelled values, a command line, and the function keys
    [DESKTOP_DOS] = {
        .background = RESOURCE_ID_IMAGE_BG_DOS,
        .fonts = {
            [FONT_CLOCK] = RESOURCE_ID_FONT_VT_84,
            [FONT_DATE] = RESOURCE_ID_FONT_VT_20,
            [FONT_TEXT] = RESOURCE_ID_FONT_VT_20,
        },
        .zones = {
            // the clock inks y14..62 and the date y68..79, clear of the panel titles on the border
            // under it at y82. the clock's rect starts above the screen
            [ZONE_TIME] = {.rect = {{4, -22}, {192, 90}}, .font_id = FONT_CLOCK, .align = GTextAlignmentCenter,
                           .color = GColorWhite},
            [ZONE_DATE] = {.rect = {{4, 59}, {192, 24}}, .font_id = FONT_DATE, .align = GTextAlignmentCenter,
                           .color = GColorIcterine},
            // the Info panel's values, one a row 14px apart, in a column at x137 that clears the
            // batt row's gauge beside its label
            [ZONE_WEATHER] = {.rect = {{137, 87}, {59, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                              .color = GColorWhite},
            [ZONE_COND] = {.rect = {{137, 101}, {59, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                           .color = GColorWhite},
            [ZONE_HR] = {.rect = {{137, 115}, {59, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                         .color = GColorWhite},
            [ZONE_STEPS] = {.rect = {{137, 129}, {59, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                            .color = GColorWhite},
        },
        .text = {
            [ZONE_HR] = hr_bpm,
        },
        // the batt row: a ten-block gauge beside the label, then the percentage in the value column
        .batt_style = BATT_STYLE_GAUGE,
        .batt_icon = {{105, 182}, {30, 8}},
        .batt_text = {{137, 171}, {59, 24}},
        .batt_align = GTextAlignmentLeft,
        .batt_text_always = true,
        .batt_font = FONT_TEXT,
        .batt_color = GColorWhite,
        .bt = {{137, 143}, {59, 24}},
        .quiet = {{137, 157}, {59, 24}},
        .status_text = true,
        .status_color = GColorScreaminGreen,
        .status_off_color = GColorLightGray,
        .quiet_color = GColorIcterine,
    },

    // a text-mode code editor: a menu bar, the clock as a line of code in the editor window, and a
    // Watch window of labelled values in two columns over a status line of function keys
    [DESKTOP_DOS_IDE] = {
        .background = RESOURCE_ID_IMAGE_BG_DOS_IDE,
        .fonts = {
            [FONT_CLOCK] = RESOURCE_ID_FONT_VT_100,
            [FONT_DATE] = RESOURCE_ID_FONT_VT_20,
            [FONT_TEXT] = RESOURCE_ID_FONT_VT_20,
        },
        .zones = {
            // the clock and date sit under the baked "char *now =", inked y49..105 and y117..128. the
            // taller @ of a .beats reading inks from y41, still clear of the code line ending at y39
            [ZONE_TIME] = {.rect = {{3, 5}, {194, 104}}, .font_id = FONT_CLOCK, .align = GTextAlignmentCenter,
                           .color = GColorIcterine},
            [ZONE_DATE] = {.rect = {{3, 108}, {194, 24}}, .font_id = FONT_DATE, .align = GTextAlignmentCenter,
                           .color = GColorWhite},
            // the Watch window's four rows, 14px apart. each value follows its baked label, with the
            // left labels at x6 and the right ones lined up at x92
            [ZONE_WEATHER] = {.rect = {{54, 144}, {34, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                              .color = GColorWhite},
            [ZONE_COND] = {.rect = {{132, 144}, {64, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                           .color = GColorWhite},
            [ZONE_HR] = {.rect = {{38, 158}, {50, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                         .color = GColorWhite},
            [ZONE_STEPS] = {.rect = {{148, 158}, {50, 24}}, .font_id = FONT_TEXT, .align = GTextAlignmentLeft,
                            .color = GColorWhite},
        },
        .text = {
            [ZONE_DATE] = date_quoted,
            [ZONE_COND] = cond_quoted,
            [ZONE_STEPS] = steps_compact,
        },
        // VT323 lands cleanly on the watch's pixels at 100 and no size between 84 and 100, but at its
        // own 40px advance the widest time is 200px, too wide for the editor. drawn a glyph at a
        // time 38px apart with a 24px colon, the widest time is 176px and keeps 9px inside the border
        .clock_pitch = 38,
        .clock_colon_pitch = 24,
        // bt: and quiet: share the third row, and batt: has the last one. no battery icon
        .batt_text = {{54, 186}, {34, 24}},
        .batt_align = GTextAlignmentLeft,
        .batt_text_always = true,
        .batt_font = FONT_TEXT,
        .batt_color = GColorWhite,
        .bt = {{38, 172}, {50, 24}},
        .quiet = {{148, 172}, {50, 24}},
        .status_text = true,
        .status_color = GColorWhite,
        .status_off_color = GColorWhite,
        .quiet_color = GColorWhite,
    },
};

const DesktopLayout *desktop_layout(uint8_t desktop)
{
    return &s_desktops[desktop < DESKTOP_COUNT ? desktop : DESKTOP_VSCODE];
}

/** @} */
