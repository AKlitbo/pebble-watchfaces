/**
 * @file text_case.c
 * @brief pure ascii case helpers implementation
 */
#include "text_case.h"

void text_to_upper(char *s)
{
    for (char *p = s; *p; p++)
    {
        if (*p >= 'a' && *p <= 'z')
        {
            *p -= 32;
        }
    }
}
