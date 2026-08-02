/**
 * @file store_cadence.c
 * @brief Holds the list of work the stores want run on the face's cadence.
 */
#include "io/stores/store_cadence.h"

static void (*s_entries[STORE_CADENCE_MAX])(void);
static int s_count;

void store_cadence_register(void (*cb)(void))
{
    if (!cb)
    {
        return;
    }

    for (int i = 0; i < s_count; i++)
    {
        if (s_entries[i] == cb)
        {
            return;
        }
    }

    // a full list means a store quietly stops getting its cadence, so the size is set by how many
    // stores there are rather than by guesswork. this fails loudly in a debug build instead
    if (s_count < STORE_CADENCE_MAX)
    {
        s_entries[s_count++] = cb;
    }
    else
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "store cadence full, dropping a registration");
    }
}

void store_cadence_fire(void)
{
    for (int i = 0; i < s_count; i++)
    {
        s_entries[i]();
    }
}
