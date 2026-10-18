#include "mosaic/draw/stat_panel.h"
#include "math/pct.h"

void stat_panel_draw_1x2(GridCtx *gctx, const StatPanel1x2 *desc)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    char val[24];
    const char *unit = NULL;
    desc->value_1x2(val, sizeof(val), &unit);

    // a dynamic icon comes back by value, so hold it while gh_stat_1x2 reads it
    IconSpec dynamic;
    const IconSpec *icon = desc->icon;
    if (desc->icon_fn)
    {
        dynamic = desc->icon_fn();
        icon = &dynamic;
    }

    gh_stat_1x2(gctx, val, unit, desc->font, icon);
}

void stat_panel_draw_goal(GridCtx *gctx, const StatPanelDesc *desc)
{
    if (gctx->size == MSIZE_1x2)
    {
        stat_panel_draw_1x2(gctx, &desc->base);
        return;
    }

    if (gctx->size == MSIZE_2x2)
    {
        char val[24];
        if (desc->value_2x2)
        {
            desc->value_2x2(val, sizeof(val));
        }
        else
        {
            // the big value is the same as the 1x2 number, minus the trailing unit we drop here
            const char *discard = NULL;
            desc->base.value_1x2(val, sizeof(val), &discard);
        }

        char goal_str[24];
        int value = 0;
        int goal = 0;
        desc->goal_2x2(goal_str, sizeof(goal_str), &value, &goal);

        gh_stat_goal_2x2(gctx, val, goal_str, desc->goal_caption, desc->base.icon, value, goal);
    }
}

void stat_panel_draw_goal_bar(GridCtx *gctx, const StatPanelDesc *desc)
{
    if (gctx->size != MSIZE_1x2 && gctx->size != MSIZE_1x4)
    {
        return;
    }

    char val[24];
    const char *unit = NULL;
    desc->base.value_1x2(val, sizeof(val), &unit);

    char goal_str[24];
    int value = 0;
    int goal = 0;
    desc->goal_2x2(goal_str, sizeof(goal_str), &value, &goal);

    // pct_of reads the no-reading marker as 0, so a panel with no reading yet shows an empty bar
    int pct = pct_of(value, goal);
    if (gctx->size == MSIZE_1x4)
    {
        gh_bar_1x4(gctx, val, unit, desc->base.icon, pct);
    }
    else
    {
        gh_bar_1x2(gctx, val, unit, desc->base.icon, pct);
    }
}
