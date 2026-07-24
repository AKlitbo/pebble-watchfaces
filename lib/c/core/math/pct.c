/**
 * @file pct.c
 * @brief Progress towards a goal, as a percent.
 */
#include "math/pct.h"

int pct_of(int value, int goal)
{
    if (goal <= 0 || value <= 0)
    {
        return 0;
    }

    int pct = (value * 100) / goal;
    return pct > 100 ? 100 : pct;
}
