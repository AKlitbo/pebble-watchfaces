/**
 * @file calendar_agenda.c
 * @brief The 2x4 Agenda panel. Lists the next few events down the body via the shared agenda-row
 * drawer, reading straight from the calendar store.
 * @ingroup gridlock_mod_calendar
 */
#include "engine/grid_engine.h"
#include "calendar_agenda.h"
#include "calendar_format.h"
#include "draw/fonts.h"
#include "io/stores/calendar_store.h"

static void calendar_agenda_body(GridCtx *gctx)
{
    if (gctx->size != MSIZE_2x4)
    {
        return;
    }

    GRect body = gctx->body;
    const CalendarStrip *strip = calendar_store_strip();

    // no reading yet so show one centred placeholder instead of an empty list
    if (strip->count == 0)
    {
        cal_draw_text(gctx, "Nothing Scheduled", FONT_STM_14, gctx->color_subtitle,
                      GRect(body.origin.x, body.origin.y + body.size.h / 2 - 8, body.size.w, 16),
                      GTextAlignmentCenter);
        return;
    }

    int rows = body.size.h / CAL_ROW_H; // however many rows fit the body
    if (rows > strip->count)
    {
        rows = strip->count;
    }

    for (int i = 0; i < rows; i++)
    {
        cal_draw_agenda_row(gctx, &strip->event[i], body, body.origin.y + i * CAL_ROW_H);
    }
}

const ModuleDef mod_calendar_agenda_def = {
    .label = "AGENDA",
    .sizes = SZ_2x4,
    .features = FEATURE_CALENDAR,
    .theme_alias = MOD_CALENDAR_COUNTDOWN, // borrow the Calendar group's colour
    .body = calendar_agenda_body
};
