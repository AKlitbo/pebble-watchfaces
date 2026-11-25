/**
 * @file header_fonts.c
 * @brief The header font table. The order matches the Header Font select in config.js, and the
 * dx/dy were tuned on-device so each font sits cleanly in the strip.
 * @ingroup mosaic_draw
 */
#include "mosaic/draw/header_fonts.h"

// index 0 is the default Share Tech Mono (a 0/0 nudge, so the header keeps its historic look).
// the rest are the fonts picked during the on-device bench, in the same order as the Clay select
static const HeaderFontSpec s_header_fonts[HEADER_FONT_COUNT] = {
    { RESOURCE_ID_FONT_STM_12,      0, 0, 0 }, // Share Tech Mono (default)
    { RESOURCE_ID_FONT_LECO_12,     0, 1, -1 }, // LECO
    { RESOURCE_ID_FONT_PRESS_8,     1, 5, -1 }, // Press Start 2P
    { RESOURCE_ID_FONT_PIXELIFY_12, 0, 0, 0 }, // Pixelify Sans
    { RESOURCE_ID_FONT_ALDRICH_11,  0, 1, 0 }, // Aldrich
    { RESOURCE_ID_FONT_KODEMONO_12, 0, 0, 0 }, // Kode Mono
    { RESOURCE_ID_FONT_ELECTRO_12,  0, 1, 0 }, // Electrolize
    { RESOURCE_ID_FONT_QUANTICO_11, 0, 1, 0 }, // Quantico
};

const HeaderFontSpec *header_font_spec(uint8_t choice)
{
    if (choice >= HEADER_FONT_COUNT)
    {
        choice = 0;
    }

    return &s_header_fonts[choice];
}

int header_label_dx(const HeaderFontSpec *spec, const char *label)
{
    if (!spec)
    {
        return 0;
    }

    return spec->dx + (label && label[0] == 'T' ? spec->t_dx : 0);
}

GFont header_font_swap(GFont old, uint8_t choice)
{
    GFont font = fonts_load_custom_font(resource_get_handle(header_font_spec(choice)->resource));

    if (old)
    {
        fonts_unload_custom_font(old);
    }

    return font;
}
