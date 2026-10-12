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
    time_t now = time(NULL);

    // nothing left to come, so show one centred placeholder instead of an empty list
    if (!cal_upcoming(0, now))
    {
        cal_draw_text(gctx, "Nothing Scheduled", FONT_STM_14, gctx->color_subtitle,
                      GRect(body.origin.x, body.origin.y + body.size.h / 2 - 8, body.size.w, 16),
                      GTextAlignmentCenter);
        return;
    }

    int rows = body.size.h / CAL_ROW_H; // however many rows fit the body

    for (int i = 0; i < rows; i++)
    {
        const CalendarEvent *event = cal_upcoming(i, now);
        if (!event)
        {
            break;
        }
        cal_draw_agenda_row(gctx, event, body, body.origin.y + i * CAL_ROW_H);
    }
}

const ModuleDef mod_calendar_agenda_def = {
    .label = "AGENDA",
    .sizes = SZ_2x4,
    .features = FEATURE_CALENDAR | FEATURE_TIME,  // the minute drops an event once it ends
    .theme_alias = MOD_CALENDAR_COUNTDOWN, // borrow the Calendar group's colour
    .body = calendar_agenda_body
};
