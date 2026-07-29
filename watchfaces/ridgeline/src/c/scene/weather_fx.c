/**
 * @file weather_fx.c
 * @brief The weather half of the scene: the recipe table plus the cloud, precipitation, fog,
 * and lightning draws.
 *
 * @ingroup watchface-ridgeline
 */
#include "weather_fx.h"

#include <string.h>

#include "scene/scene.h"

/**
 * @addtogroup watchface-ridgeline
 * @{
 */

// one row per condition token the phone can send, in the order lib/ts/weather/conditions.ts
// lists them. the tokens are the wire spelling and must match it exactly
typedef struct
{
    const char *token;
    SkyRecipe   recipe;
} ConditionRow;

static const ConditionRow s_rows[] = {
    {"CLEAR", {.clouds = CLOUDS_NONE,      .celestial = CEL_FULL,   .precip = PRECIP_NONE}},
    {"PCLDY", {.clouds = CLOUDS_DRIFTING,  .celestial = CEL_PEEK,   .precip = PRECIP_NONE}},
    {"CLDY",  {.clouds = CLOUDS_OVERCAST,  .celestial = CEL_HIDDEN, .precip = PRECIP_NONE}},
    {"FOGGY", {.clouds = CLOUDS_DRIFTING,  .celestial = CEL_HIDDEN, .precip = PRECIP_NONE, .fog = true}},
    {"DRZL",  {.clouds = CLOUDS_DRIFTING,  .celestial = CEL_PEEK,   .precip = PRECIP_DROPS,  .drops = 16}},
    {"FZDZ",  {.clouds = CLOUDS_OVERCAST,  .celestial = CEL_HIDDEN, .precip = PRECIP_MIXED,  .drops = 18}},
    {"RAIN",  {.clouds = CLOUDS_OVERCAST,  .celestial = CEL_HIDDEN, .precip = PRECIP_DASHES, .drops = 28}},
    {"FZRN",  {.clouds = CLOUDS_OVERCAST,  .celestial = CEL_HIDDEN, .precip = PRECIP_MIXED,  .drops = 24}},
    {"SNOW",  {.clouds = CLOUDS_OVERCAST,  .celestial = CEL_HIDDEN, .precip = PRECIP_FLAKES, .drops = 22, .snowcap = true}},
    // a shower is a passing cloud, so the sun stays in shot behind it
    {"SHWR",  {.clouds = CLOUDS_DRIFTING,  .celestial = CEL_PEEK,   .precip = PRECIP_DASHES, .drops = 20}},
    {"SNSH",  {.clouds = CLOUDS_DRIFTING,  .celestial = CEL_PEEK,   .precip = PRECIP_FLAKES, .drops = 18, .snowcap = true}},
    {"STRM",  {.clouds = CLOUDS_OVERCAST,  .celestial = CEL_HIDDEN, .precip = PRECIP_DASHES, .drops = 30, .bolt = true}},
};

SkyRecipe fx_recipe(const char *condition)
{
    SkyRecipe open_sky = {.clouds = CLOUDS_NONE, .celestial = CEL_FULL, .precip = PRECIP_NONE};

    if (!condition || !condition[0])
    {
        return open_sky;
    }

    // "_NIGHT" rides on the token but says nothing about the sky, so compare up to it
    size_t len = strlen(condition);
    if (len > 6 && !strcmp(condition + len - 6, "_NIGHT"))
    {
        len -= 6;
    }

    for (unsigned i = 0; i < ARRAY_LENGTH(s_rows); i++)
    {
        if (strlen(s_rows[i].token) == len && !strncmp(s_rows[i].token, condition, len))
        {
            return s_rows[i].recipe;
        }
    }

    return open_sky;
}

/** @brief One puff of a cloud: where its base sits, how wide it is, and how big its lobes are. */
typedef struct
{
    int16_t x;      ///< Bottom-centre
    int16_t y;      ///< Base line
    uint8_t half_w; ///< Half the base width
    uint8_t lobe;   ///< Radius of the outer lobes
} Puff;

// the overcast band: puffs spaced closer than their own width so they run into each other and
// come out as one lumpy mass rather than a row of separate clouds. the base line rises and
// falls across the sky so the underside is not a ruled edge
// a puff reaches (2 * lobe + 11) above its base once the tallest lobe and the ink pass are
// counted, so every base here is set low enough to keep the tops clear of the status bar
static const Puff s_overcast[] = {
    {14, 60, 26, 10}, {58, 56, 24, 11}, {104, 63, 27, 10}, {150, 57, 25, 11}, {192, 64, 22, 9},
};

/**
 * @brief Draw a group of puffs as one cloud.
 *
 * Every puff is laid down twice: once a couple of pixels fatter in the ink colour across the
 * whole group, then once at size in the cloud colour. Outlining the group rather than each
 * puff is what merges overlapping puffs into a single fluffy silhouette. Outline each puff on
 * its own and every seam shows straight through the middle.
 *
 * @param ctx The graphics context.
 * @param pal The palette in use.
 * @param puffs The puffs making up the cloud.
 * @param count How many puffs there are.
 */
static void draw_puffs(GContext *ctx, const Palette *pal, const Puff *puffs, int count)
{
    for (int pass = 0; pass < 2; pass++)
    {
        int grow = pass == 0 ? 2 : 0;
        graphics_context_set_fill_color(ctx, pass == 0 ? pal->ink : pal->cloud);

        for (int i = 0; i < count; i++)
        {
            int x = puffs[i].x;
            int y = puffs[i].y;
            int half_w = puffs[i].half_w;
            int lobe = puffs[i].lobe;

            // five bumps at four different heights, tallest just off-centre, so the top edge
            // reads as fluff instead of a row of matching humps
            graphics_fill_circle(ctx, GPoint(x - half_w + lobe, y - lobe + 2), lobe + grow);
            graphics_fill_circle(ctx, GPoint(x - half_w / 2, y - lobe - 3), lobe + 1 + grow);
            graphics_fill_circle(ctx, GPoint(x, y - lobe - 6), lobe + 3 + grow);
            graphics_fill_circle(ctx, GPoint(x + half_w / 2, y - lobe - 2), lobe + grow);
            graphics_fill_circle(ctx, GPoint(x + half_w - lobe, y - lobe + 1), lobe - 1 + grow);
            graphics_fill_rect(ctx, GRect(x - half_w - grow, y - lobe, half_w * 2 + grow * 2, lobe + grow), 0, GCornerNone);
        }
    }
}

void fx_draw_clouds(GContext *ctx, const Palette *pal, const SkyRecipe *recipe, GPoint disc)
{
    if (recipe->clouds == CLOUDS_NONE)
    {
        return;
    }

    if (recipe->clouds == CLOUDS_OVERCAST)
    {
        draw_puffs(ctx, pal, s_overcast, ARRAY_LENGTH(s_overcast));
        return;
    }

    // a single drifting cloud, parked on the disc so a peeking sun always reads as
    // half-covered, and kept clear of the edges wherever the disc has got to. two puffs so
    // even the lone cloud has some fluff to it
    int x = disc.x + 14;
    if (x < 52)
    {
        x = 52;
    }
    if (x > 146)
    {
        x = 146;
    }

    // the low end of the clamp is what keeps the taller lobe out from under the status bar when
    // the sun is high; the high end keeps the cloud off the ridges when it is low
    int y = disc.y + 20;
    if (y < 57)
    {
        y = 57;
    }
    if (y > 82)
    {
        y = 82;
    }

    const Puff drifting[] = {
        {x - 16, y, 18, 9},
        {x + 14, y - 4, 20, 10},
    };
    draw_puffs(ctx, pal, drifting, ARRAY_LENGTH(drifting));
}

/**
 * @brief Draw one falling particle.
 *
 * @param ctx The graphics context.
 * @param kind A PrecipKind.
 * @param at Where the particle sits.
 * @param index The particle's number, which picks the half of a mixed fall.
 */
static void draw_particle(GContext *ctx, uint8_t kind, GPoint at, int index)
{
    if (kind == PRECIP_MIXED)
    {
        kind = (index & 1) ? PRECIP_DROPS : PRECIP_DASHES;
    }

    if (kind == PRECIP_DASHES)
    {
        graphics_draw_line(ctx, at, GPoint(at.x - 2, at.y + 6));
        return;
    }

    if (kind == PRECIP_FLAKES)
    {
        graphics_draw_line(ctx, GPoint(at.x - 2, at.y), GPoint(at.x + 2, at.y));
        graphics_draw_line(ctx, GPoint(at.x, at.y - 2), GPoint(at.x, at.y + 2));
        return;
    }

    graphics_fill_circle(ctx, at, 1);
}

void fx_draw_precip(GContext *ctx, const Palette *pal, const SkyRecipe *recipe, int minute)
{
    if (recipe->precip == PRECIP_NONE)
    {
        return;
    }

    graphics_context_set_stroke_color(ctx, pal->precip);
    graphics_context_set_fill_color(ctx, pal->precip);

    // only odd stroke widths render, so rain gets the fat one and everything finer stays at 1
    graphics_context_set_stroke_width(ctx, recipe->precip == PRECIP_DASHES ? 3 : 1);

    // the two strides are coprime with the span, so the particles spread out instead of
    // lining up, and the minute slides the whole field along
    for (int i = 0; i < recipe->drops; i++)
    {
        int x = 6 + (i * 41 + minute * 13) % 188;
        int y = 52 + (i * 27 + minute * 19) % 70;
        draw_particle(ctx, recipe->precip, GPoint(x, y), i);
    }

    graphics_context_set_stroke_width(ctx, 1);
}

void fx_draw_fog(GContext *ctx, const Palette *pal)
{
    // the watch has no alpha, so the mist is stippled instead: rows of cloud-coloured dots
    // over the ridges, packed tight low down and thinning as they rise. drawn as a wash
    // rather than as banks because solid lines read as bars ruled across the mountains
    graphics_context_set_stroke_color(ctx, pal->cloud);

    for (int y = FOG_TOP; y < FOG_BOTTOM; y += 2)
    {
        int step = 9 - (y - FOG_TOP) / 6;
        if (step < 2)
        {
            step = 2;
        }

        // the row's start shifts with y so the dots never stack into vertical stripes
        for (int x = (y / 2) % step; x < 200; x += step)
        {
            graphics_draw_pixel(ctx, GPoint(x, y));
        }
    }

    // a couple of solid wisps through the densest part, which gives the wash an edge to read
    // against and stops it looking like flat noise
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_line(ctx, GPoint(18, 112), GPoint(74, 112));
    graphics_draw_line(ctx, GPoint(96, 118), GPoint(158, 118));
    graphics_context_set_stroke_width(ctx, 1);
}

void fx_draw_bolt(GContext *ctx, const Palette *pal)
{
    // a zigzag out of the cloud base, drawn as lines rather than a path so it needs no
    // allocation and keeps the same hand-drawn weight as the rest of the linework
    static const GPoint stroke[][2] = {
        {{116, 58}, {98, 86}},
        {{98, 86}, {114, 84}},
        {{114, 84}, {88, 120}},
    };

    // laid down fat in ink first, then narrower in the disc colour, so the fork keeps an
    // outline and stays legible against a pale sky and through the rain in front of it
    for (int pass = 0; pass < 2; pass++)
    {
        graphics_context_set_stroke_color(ctx, pass == 0 ? pal->ink : pal->disc);
        graphics_context_set_stroke_width(ctx, pass == 0 ? 7 : 3);

        for (unsigned i = 0; i < ARRAY_LENGTH(stroke); i++)
        {
            graphics_draw_line(ctx, stroke[i][0], stroke[i][1]);
        }
    }

    graphics_context_set_stroke_width(ctx, 1);
}

/** @} */
