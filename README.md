# Pebble Watchfaces

A collection of watchfaces for current Pebble hardware, built on one shared engine. Each face shows the time, date, weather and battery, wrapped in its own interface, with themes selectable from a Clay settings page. Most also show heart rate and steps where the watch has the sensors for them.

## Watchfaces

### Standalone

| Watchface | Preview |
| :--- | :--- |
| **LCARS Stardate**<br>[readouts](watchfaces/lcars-stardate/MODULES.md) · [changelog](watchfaces/lcars-stardate/CHANGELOG.md) | <img src=".github/images/lcars-stardate/theme_classic.png" width="75" title="Classic"> <img src=".github/images/lcars-stardate/theme_nemesis-blue.png" width="75" title="Nemesis Blue"> <img src=".github/images/lcars-stardate/theme_mono.png" width="75" title="Classic Mono"> <img src=".github/images/lcars-stardate/theme_voyager.png" width="75" title="Voyager"> <img src=".github/images/lcars-stardate/theme_voyager-mono.png" width="75" title="Voyager Mono"> <img src=".github/images/lcars-stardate/theme_lower-decks.png" width="75" title="Lower Decks"> <img src=".github/images/lcars-stardate/theme_lower-decks-mono.png" width="75" title="Lower Decks Mono"> <img src=".github/images/lcars-stardate/theme_lower-decks-padd.png" width="75" title="Lower Decks PADD"> <img src=".github/images/lcars-stardate/theme_lower-decks-padd-mono.png" width="75" title="Lower Decks PADD Mono"> |
| **Radar Array**<br>[changelog](watchfaces/radar-array/CHANGELOG.md) | <img src=".github/images/radar-array/theme_default.png" width="75" title="Default"> <img src=".github/images/radar-array/theme_crimson.png" width="75" title="Crimson"> <img src=".github/images/radar-array/theme_neon.png" width="75" title="Neon"> <img src=".github/images/radar-array/theme_phosphor.png" width="75" title="Phosphor"> <img src=".github/images/radar-array/theme_rescue.png" width="75" title="Rescue"> <img src=".github/images/radar-array/theme_stealth.png" width="75" title="Stealth"> <img src=".github/images/radar-array/theme_mono.png" width="75" title="Mono"> |
| **IDE VSCode**<br>[changelog](watchfaces/ide-vscode/CHANGELOG.md) | <img src=".github/images/ide-vscode/theme_dark.png" width="75" title="Dark"> <img src=".github/images/ide-vscode/theme_light.png" width="75" title="Light"> <img src=".github/images/ide-vscode/theme_terminal.png" width="75" title="Terminal"> <img src=".github/images/ide-vscode/theme_cyberpunk.png" width="75" title="Cyberpunk"> <img src=".github/images/ide-vscode/theme_synthwave.png" width="75" title="Synthwave"> <img src=".github/images/ide-vscode/theme_mono.png" width="75" title="Mono"> |

### Mosaic

Assembled rather than themed, out of one shared catalogue of [panels](watchfaces/mosaic/MODULES.md). Gridlock has no fixed screen, so its previews are layouts rather than colourways, built with the per-panel colours and the header and border toggles the settings page offers. You can build four layouts rather than one and assign two as your day and night screens. Gridlock swaps between them at sunrise and sunset or at times you set.

Sidereel has no presets either. Every colour is drawn rather than baked, so the settings page hands you a swatch grid per part of the face and the watch's full palette to pick from. The two halves down its left side each take a stacked pair of panels or a single tall one, and the day wraps the outside edge as a track with the daylight hours shaded and a marker riding round at the current time.

| Watchface | Preview |
| :--- | :--- |
| **Gridlock**<br>[changelog](watchfaces/mosaic/gridlock/CHANGELOG.md) | <img src=".github/images/gridlock/layout_everyday.png" width="75" title="Everyday"> <img src=".github/images/gridlock/layout_big-clock.png" width="75" title="Big clock"> <img src=".github/images/gridlock/layout_analog.png" width="75" title="Analog"> <img src=".github/images/gridlock/layout_weather.png" width="75" title="Weather station"> <img src=".github/images/gridlock/layout_training.png" width="75" title="Training"> <img src=".github/images/gridlock/layout_agenda.png" width="75" title="Agenda"> <img src=".github/images/gridlock/layout_stocks.png" width="75" title="Stocks"> <img src=".github/images/gridlock/layout_minimal.png" width="75" title="Minimal"><br><img src=".github/images/gridlock/layout_sun-moon.png" width="75" title="Sun and moon"> <img src=".github/images/gridlock/layout_dense.png" width="75" title="Dense"> <img src=".github/images/gridlock/layout_forecast.png" width="75" title="Forecast"> <img src=".github/images/gridlock/layout_time-units.png" width="75" title="Time units"> <img src=".github/images/gridlock/layout_two-clocks.png" width="75" title="Two clocks"> <img src=".github/images/gridlock/layout_recovery.png" width="75" title="Recovery"> <img src=".github/images/gridlock/layout_month.png" width="75" title="Month"> <img src=".github/images/gridlock/layout_hot.png" width="75" title="Hot"><br><img src=".github/images/gridlock/layout_everyday-bare.png" width="75" title="Everyday, bare"> <img src=".github/images/gridlock/layout_cold.png" width="75" title="Cold"> <img src=".github/images/gridlock/layout_big-hour.png" width="75" title="Big hour"> <img src=".github/images/gridlock/layout_forecast-pair.png" width="75" title="Forecast pair"> <img src=".github/images/gridlock/layout_agenda-bare.png" width="75" title="Agenda, bare"> <img src=".github/images/gridlock/layout_neon.png" width="75" title="Neon"> <img src=".github/images/gridlock/layout_dial.png" width="75" title="Dial"> <img src=".github/images/gridlock/layout_health.png" width="75" title="Health"><br><img src=".github/images/gridlock/layout_watchlist.png" width="75" title="Watchlist"> <img src=".github/images/gridlock/layout_mixed.png" width="75" title="Mixed"> <img src=".github/images/gridlock/layout_daylight.png" width="75" title="Daylight"> <img src=".github/images/gridlock/layout_month-bare.png" width="75" title="Month, bare"> <img src=".github/images/gridlock/layout_terminal.png" width="75" title="Terminal"> <img src=".github/images/gridlock/layout_beats.png" width="75" title="Beats"> <img src=".github/images/gridlock/layout_timeline.png" width="75" title="Timeline"> <img src=".github/images/gridlock/layout_dusk.png" width="75" title="Dusk"> |
| **Sidereel**<br>[changelog](watchfaces/mosaic/sidereel/CHANGELOG.md) | <img src=".github/images/sidereel/layout_everyday.png" width="75" title="Everyday"> <img src=".github/images/sidereel/layout_weather.png" width="75" title="Weather station"> <img src=".github/images/sidereel/layout_health.png" width="75" title="Health graphs"> <img src=".github/images/sidereel/layout_sun-moon.png" width="75" title="Sun and moon"> <img src=".github/images/sidereel/layout_daylight.png" width="75" title="Daylight"> <img src=".github/images/sidereel/layout_training.png" width="75" title="Training"> <img src=".github/images/sidereel/layout_recovery.png" width="75" title="Recovery"> <img src=".github/images/sidereel/layout_forecast.png" width="75" title="Forecast"><br><img src=".github/images/sidereel/layout_two-zones.png" width="75" title="Two zones"> <img src=".github/images/sidereel/layout_calendar.png" width="75" title="Calendar"> <img src=".github/images/sidereel/layout_time-units.png" width="75" title="Time units"> <img src=".github/images/sidereel/layout_dense.png" width="75" title="Dense"> <img src=".github/images/sidereel/layout_minimal.png" width="75" title="Minimal"> <img src=".github/images/sidereel/layout_bare.png" width="75" title="Bare"> <img src=".github/images/sidereel/layout_rounded.png" width="75" title="Rounded panels"> <img src=".github/images/sidereel/layout_rounded-plain.png" width="75" title="Rounded, no headers"><br><img src=".github/images/sidereel/layout_inverse.png" width="75" title="Inverse"> <img src=".github/images/sidereel/layout_inverse-bare.png" width="75" title="Inverse, bare"> <img src=".github/images/sidereel/layout_hot.png" width="75" title="Hot"> <img src=".github/images/sidereel/layout_cold.png" width="75" title="Cold"> <img src=".github/images/sidereel/layout_neon.png" width="75" title="Neon"> <img src=".github/images/sidereel/layout_terminal.png" width="75" title="Terminal"> <img src=".github/images/sidereel/layout_amber.png" width="75" title="Amber"> <img src=".github/images/sidereel/layout_mixed.png" width="75" title="Mixed"> |

### Sketchbook

These run on both the Pebble Time 2 (**Emery**) and the Round 2 (**Gabbro**). On the round screen they carry their reading in the scene rather than in a row of stats: on the boat's pennant, on a trail sign, on the cabin.

| Watchface | Preview |
| :--- | :--- |
| **Ridgeline**<br>[changelog](watchfaces/sketchbook/ridgeline/CHANGELOG.md) | **Emery**<br><img src=".github/images/ridgeline/emery_sketchbook.png" width="75" title="Sketchbook (Day)"> <img src=".github/images/ridgeline/emery_daybreak.png" width="75" title="Daybreak (Day)"> <img src=".github/images/ridgeline/emery_alpenglow.png" width="75" title="Alpenglow (Day)"> <img src=".github/images/ridgeline/emery_blueprint.png" width="75" title="Blueprint (Day)"> <img src=".github/images/ridgeline/emery_forest.png" width="75" title="Forest (Day)"> <img src=".github/images/ridgeline/emery_mono.png" width="75" title="Mono (Day)"> <img src=".github/images/ridgeline/emery_cyberpunk.png" width="75" title="Cyberpunk (Day)"> <img src=".github/images/ridgeline/emery_neo-tokyo.png" width="75" title="Neo Tokyo (Day)"><br><img src=".github/images/ridgeline/emery_sketchbook_night.png" width="75" title="Sketchbook (Night)"> <img src=".github/images/ridgeline/emery_daybreak_night.png" width="75" title="Daybreak (Night)"> <img src=".github/images/ridgeline/emery_alpenglow_night.png" width="75" title="Alpenglow (Night)"> <img src=".github/images/ridgeline/emery_blueprint_night.png" width="75" title="Blueprint (Night)"> <img src=".github/images/ridgeline/emery_forest_night.png" width="75" title="Forest (Night)"> <img src=".github/images/ridgeline/emery_mono_night.png" width="75" title="Mono (Night)"> <img src=".github/images/ridgeline/emery_cyberpunk_night.png" width="75" title="Cyberpunk (Night)"> <img src=".github/images/ridgeline/emery_neo-tokyo_night.png" width="75" title="Neo Tokyo (Night)"><br>**Gabbro**<br><img src=".github/images/ridgeline/gabbro_sketchbook.png" width="75" title="Sketchbook (Day)"> <img src=".github/images/ridgeline/gabbro_daybreak.png" width="75" title="Daybreak (Day)"> <img src=".github/images/ridgeline/gabbro_alpenglow.png" width="75" title="Alpenglow (Day)"> <img src=".github/images/ridgeline/gabbro_blueprint.png" width="75" title="Blueprint (Day)"> <img src=".github/images/ridgeline/gabbro_forest.png" width="75" title="Forest (Day)"> <img src=".github/images/ridgeline/gabbro_mono.png" width="75" title="Mono (Day)"> <img src=".github/images/ridgeline/gabbro_cyberpunk.png" width="75" title="Cyberpunk (Day)"> <img src=".github/images/ridgeline/gabbro_neo-tokyo.png" width="75" title="Neo Tokyo (Day)"><br><img src=".github/images/ridgeline/gabbro_sketchbook_night.png" width="75" title="Sketchbook (Night)"> <img src=".github/images/ridgeline/gabbro_daybreak_night.png" width="75" title="Daybreak (Night)"> <img src=".github/images/ridgeline/gabbro_alpenglow_night.png" width="75" title="Alpenglow (Night)"> <img src=".github/images/ridgeline/gabbro_blueprint_night.png" width="75" title="Blueprint (Night)"> <img src=".github/images/ridgeline/gabbro_forest_night.png" width="75" title="Forest (Night)"> <img src=".github/images/ridgeline/gabbro_mono_night.png" width="75" title="Mono (Night)"> <img src=".github/images/ridgeline/gabbro_cyberpunk_night.png" width="75" title="Cyberpunk (Night)"> <img src=".github/images/ridgeline/gabbro_neo-tokyo_night.png" width="75" title="Neo Tokyo (Night)"> |
| **Shoreline**<br>[changelog](watchfaces/sketchbook/shoreline/CHANGELOG.md) | **Emery**<br><img src=".github/images/shoreline/emery_sketchbook.png" width="75" title="Sketchbook (Day)"> <img src=".github/images/shoreline/emery_daybreak.png" width="75" title="Daybreak (Day)"> <img src=".github/images/shoreline/emery_alpenglow.png" width="75" title="Alpenglow (Day)"> <img src=".github/images/shoreline/emery_blueprint.png" width="75" title="Blueprint (Day)"> <img src=".github/images/shoreline/emery_forest.png" width="75" title="Forest (Day)"> <img src=".github/images/shoreline/emery_mono.png" width="75" title="Mono (Day)"> <img src=".github/images/shoreline/emery_cyberpunk.png" width="75" title="Cyberpunk (Day)"> <img src=".github/images/shoreline/emery_neo-tokyo.png" width="75" title="Neo Tokyo (Day)"><br><img src=".github/images/shoreline/emery_sketchbook_night.png" width="75" title="Sketchbook (Night)"> <img src=".github/images/shoreline/emery_daybreak_night.png" width="75" title="Daybreak (Night)"> <img src=".github/images/shoreline/emery_alpenglow_night.png" width="75" title="Alpenglow (Night)"> <img src=".github/images/shoreline/emery_blueprint_night.png" width="75" title="Blueprint (Night)"> <img src=".github/images/shoreline/emery_forest_night.png" width="75" title="Forest (Night)"> <img src=".github/images/shoreline/emery_mono_night.png" width="75" title="Mono (Night)"> <img src=".github/images/shoreline/emery_cyberpunk_night.png" width="75" title="Cyberpunk (Night)"> <img src=".github/images/shoreline/emery_neo-tokyo_night.png" width="75" title="Neo Tokyo (Night)"><br>**Gabbro**<br><img src=".github/images/shoreline/gabbro_sketchbook.png" width="75" title="Sketchbook (Day)"> <img src=".github/images/shoreline/gabbro_daybreak.png" width="75" title="Daybreak (Day)"> <img src=".github/images/shoreline/gabbro_alpenglow.png" width="75" title="Alpenglow (Day)"> <img src=".github/images/shoreline/gabbro_blueprint.png" width="75" title="Blueprint (Day)"> <img src=".github/images/shoreline/gabbro_forest.png" width="75" title="Forest (Day)"> <img src=".github/images/shoreline/gabbro_mono.png" width="75" title="Mono (Day)"> <img src=".github/images/shoreline/gabbro_cyberpunk.png" width="75" title="Cyberpunk (Day)"> <img src=".github/images/shoreline/gabbro_neo-tokyo.png" width="75" title="Neo Tokyo (Day)"><br><img src=".github/images/shoreline/gabbro_sketchbook_night.png" width="75" title="Sketchbook (Night)"> <img src=".github/images/shoreline/gabbro_daybreak_night.png" width="75" title="Daybreak (Night)"> <img src=".github/images/shoreline/gabbro_alpenglow_night.png" width="75" title="Alpenglow (Night)"> <img src=".github/images/shoreline/gabbro_blueprint_night.png" width="75" title="Blueprint (Night)"> <img src=".github/images/shoreline/gabbro_forest_night.png" width="75" title="Forest (Night)"> <img src=".github/images/shoreline/gabbro_mono_night.png" width="75" title="Mono (Night)"> <img src=".github/images/shoreline/gabbro_cyberpunk_night.png" width="75" title="Cyberpunk (Night)"> <img src=".github/images/shoreline/gabbro_neo-tokyo_night.png" width="75" title="Neo Tokyo (Night)"> |
| **Treeline**<br>[changelog](watchfaces/sketchbook/treeline/CHANGELOG.md) | **Emery**<br><img src=".github/images/treeline/emery_sketchbook.png" width="75" title="Sketchbook (Day)"> <img src=".github/images/treeline/emery_daybreak.png" width="75" title="Daybreak (Day)"> <img src=".github/images/treeline/emery_alpenglow.png" width="75" title="Alpenglow (Day)"> <img src=".github/images/treeline/emery_blueprint.png" width="75" title="Blueprint (Day)"> <img src=".github/images/treeline/emery_forest.png" width="75" title="Forest (Day)"> <img src=".github/images/treeline/emery_mono.png" width="75" title="Mono (Day)"> <img src=".github/images/treeline/emery_cyberpunk.png" width="75" title="Cyberpunk (Day)"> <img src=".github/images/treeline/emery_neo-tokyo.png" width="75" title="Neo Tokyo (Day)"><br><img src=".github/images/treeline/emery_sketchbook_night.png" width="75" title="Sketchbook (Night)"> <img src=".github/images/treeline/emery_daybreak_night.png" width="75" title="Daybreak (Night)"> <img src=".github/images/treeline/emery_alpenglow_night.png" width="75" title="Alpenglow (Night)"> <img src=".github/images/treeline/emery_blueprint_night.png" width="75" title="Blueprint (Night)"> <img src=".github/images/treeline/emery_forest_night.png" width="75" title="Forest (Night)"> <img src=".github/images/treeline/emery_mono_night.png" width="75" title="Mono (Night)"> <img src=".github/images/treeline/emery_cyberpunk_night.png" width="75" title="Cyberpunk (Night)"> <img src=".github/images/treeline/emery_neo-tokyo_night.png" width="75" title="Neo Tokyo (Night)"><br>**Gabbro**<br><img src=".github/images/treeline/gabbro_sketchbook.png" width="75" title="Sketchbook (Day)"> <img src=".github/images/treeline/gabbro_daybreak.png" width="75" title="Daybreak (Day)"> <img src=".github/images/treeline/gabbro_alpenglow.png" width="75" title="Alpenglow (Day)"> <img src=".github/images/treeline/gabbro_blueprint.png" width="75" title="Blueprint (Day)"> <img src=".github/images/treeline/gabbro_forest.png" width="75" title="Forest (Day)"> <img src=".github/images/treeline/gabbro_mono.png" width="75" title="Mono (Day)"> <img src=".github/images/treeline/gabbro_cyberpunk.png" width="75" title="Cyberpunk (Day)"> <img src=".github/images/treeline/gabbro_neo-tokyo.png" width="75" title="Neo Tokyo (Day)"><br><img src=".github/images/treeline/gabbro_sketchbook_night.png" width="75" title="Sketchbook (Night)"> <img src=".github/images/treeline/gabbro_daybreak_night.png" width="75" title="Daybreak (Night)"> <img src=".github/images/treeline/gabbro_alpenglow_night.png" width="75" title="Alpenglow (Night)"> <img src=".github/images/treeline/gabbro_blueprint_night.png" width="75" title="Blueprint (Night)"> <img src=".github/images/treeline/gabbro_forest_night.png" width="75" title="Forest (Night)"> <img src=".github/images/treeline/gabbro_mono_night.png" width="75" title="Mono (Night)"> <img src=".github/images/treeline/gabbro_cyberpunk_night.png" width="75" title="Cyberpunk (Night)"> <img src=".github/images/treeline/gabbro_neo-tokyo_night.png" width="75" title="Neo Tokyo (Night)"> |

## Install

Download a face's `.pbw` from [Releases](https://github.com/AKlitbo/pebble-watchfaces/releases) and open it with the Pebble app on your phone.

Faces version independently, so releases are tagged per face as `<face>-v<version>`. Release notes are that version's `CHANGELOG.md` entry. A face built for a single watch names its platform in the asset, so `lcars-stardate-emery-1.7.0.pbw` is Emery only. A face that runs on more than one is named by version alone, since naming one watch would not be true of it. `ridgeline-1.2.0.pbw` holds a build for the Pebble Time 2 and one for the Round 2, and installs on both.

Most faces carry one `.pbw`. Gridlock carries two, a watchface and a watchapp built from the same source. They share a UUID, so only one can be on the watch at a time: the watchface build sits in your watchface carousel and is the one on the appstore, and the watchapp build lives in the launcher instead. Take whichever you want from the releases page.

## Project Structure

The shared engine lives at the root. Each face owns only what makes it that face.

A face is any directory carrying a `config/pebble.appinfo.json`, so it is found whether it sits at `watchfaces/<face>/` or inside a family at `watchfaces/<family>/<face>/`. A family is a group of related faces plus the code only they share.

* **`watchfaces/<face>/`**: one face. `config/` holds its identity (uuid, version, message keys, resources), `src/c/` the device code, `src/pkjs/` the Clay config page and phone-side bridge, `resources/` its fonts and PNGs, and `CHANGELOG.md` its own release history. Some also carry a `frame/`, the HTML the backgrounds are baked from, or a `src/tools/` of generators only that face uses.
* **`watchfaces/<family>/core/`**: the family's shared code, staged into each member's build and reached as `<family>/...`.
* **`lib/`**: the base every face shares (`c/` device engine, `ts/` PebbleKit JS, `py/` waf helpers, `tools/` generators, `css/` the Pebble-64 gamut, `testing/` test helpers).
* **`tools/`** and **`config/`**: build tooling, and the shared tsconfig/eslint/vitest setup.
* **`targets/<target>/`**: the build sandbox waf runs in, generated and gitignored. Usually `targets/<face>/`, unless the face declares a `targets` map in its appinfo and gets one sandbox per target.
* **`vendor/`**: third-party source SVGs and the LCARS template (gitignored, see [Third-Party Assets](#third-party-assets)).
* **`build.sh`**: regenerates a face's manifest, compiles its pkjs, and runs `pebble build`.

Anything with a `.g.` in the name is generated and should not be hand-edited: rerun the matching `npm run gen:*`. CI checks that the committed output still matches.

### Adding a Face

Create `watchfaces/<name>/` with the layout above, then build it. The sandbox, manifest, and waf entry point are all generated from the face's name and appinfo. Add `<name>` to the `face` matrix in [.github/workflows/ci.yml](.github/workflows/ci.yml) so it builds on every push.

To join a family instead, create it at `watchfaces/<family>/<name>/`. Nothing else changes, and the family's `core/` is compiled in automatically because of where the face sits.

## Releasing

Pushing a `<face>-v<version>` tag is the whole process. [release.yml](.github/workflows/release.yml) builds that face, takes its notes from the matching `CHANGELOG.md` section, and publishes the `.pbw`.

```sh
# date the [1.5.0] heading in watchfaces/lcars-stardate/CHANGELOG.md first, then
git tag lcars-stardate-v1.5.0
git push origin lcars-stardate-v1.5.0
```

The tag version must match `version` in that face's `config/pebble.appinfo.json`, the changelog entry must be dated, and the tag must not already be released. The workflow checks all three before it spends time on a build, so a mistake costs seconds.

## Development

```sh
npm ci
git config core.hooksPath .githooks   # once: runs lint + typecheck before each commit
./build.sh lcars-stardate             # the .pbw, from WSL with the Pebble SDK installed
```

Every face-scoped command takes the face name:

```sh
./build.sh <face> [--clean]           # build a .pbw into targets/<face>/build/
npm run build:pkjs -- <face>          # compile src/pkjs + lib/ts into targets/<face>/emit/
npm run build:manifests -- <face>     # regenerate the waf manifest + wscript
npm run gen:icons -- <face>           # rasterize vendored SVGs to resources/icons/*.png
npm run gen:frame -- <face> [theme]   # re-bake a background from frame/<name>.html
```

Repo-wide checks cover `lib/`, `tools/`, and every face:

```sh
npm test          # offline unit suite
npm run lint
npm run typecheck
```

## Weather Providers

Selectable in Settings:

- **Open-Meteo** *(recommended)*: free, no account or API key.
- **WeatherAPI**: free tier, needs an account and API key.
- **OpenWeatherMap**: free tier, needs an account and API key.

All cover the basics: temperature, conditions, wind, humidity, pressure, feels like, and sunrise/sunset. OpenWeatherMap's free tier leaves out UV index, dew point, today's high/low, and chance of rain, so those are backfilled from Open-Meteo.

## Calendar (iCal)

On a face that supports it, calendar comes from a private iCal (`.ics`) feed, so any service that publishes one works (Google Calendar's secret address is the usual choice). Paste the feed URL into Settings and upcoming events are fetched automatically.

Only event time, title, and location are sent to the watch, and events more than a week away are ignored.

Feeds are parsed with [ical.js](https://github.com/kewisch/ical.js), including recurring events, exceptions, cancellations, and time zones. It is licensed under MPL 2.0 and included as a separate file. See [NOTICES](NOTICES.md).

Alarms and attendees are currently ignored.

## Stock Providers

On a face that supports it, the providers below are selectable in Settings. All are free, and all except Yahoo need an account and API key. Set your ticker(s) and key there too.

- **Finnhub** *(recommended)*: real-time US quotes, follows your refresh interval.
- **Yahoo**: real-time, no API key, and the widest coverage (US and international stocks, ETFs, indices, and crypto). Unofficial feed, so it can break without notice.
- **Twelve Data**: global markets. Follows your interval while markets are open, no faster than every 15 minutes, then slows down after hours to stay within daily limits.
- **Alpha Vantage**: one end-of-day quote after the close, with a small daily request allowance.

All return the same core quote data: last price, price change, percentage change, and latest trading day.

---

## Credits

* **LCARS Stardate**
  * **LCARS Design**: LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications.
  * **Typography**: [Antonio](https://fonts.google.com/specimen/Antonio).
  * **Glyphs**: Heart, step, thermometer, and muted-speaker icons from [UXWing](https://uxwing.com).
* **Radar Array**
  * **Typography**: [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono).
* **IDE VSCode**
  * **Typography**: [Teko](https://fonts.google.com/specimen/Teko) and [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono).
* **Mosaic Faces**
  * **Typography**: [Teko](https://fonts.google.com/specimen/Teko) and [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono) for the clock and values, with [LECO 2014](https://www.1001fonts.com/leco-2014-font.html), [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P), [Pixelify Sans](https://fonts.google.com/specimen/Pixelify+Sans), [Aldrich](https://fonts.google.com/specimen/Aldrich), [Kode Mono](https://fonts.google.com/specimen/Kode+Mono), [Electrolize](https://fonts.google.com/specimen/Electrolize), and [Quantico](https://fonts.google.com/specimen/Quantico) as header options.
  * **Glyphs**: Heart, step, distance, thermometer, UV, fire, snooze, late, clock, globe, calendar, and volume icons from [UXWing](https://uxwing.com).
* **Sketchbook Faces**
  * **Typography**: [Patrick Hand](https://fonts.google.com/specimen/Patrick+Hand).
  * **Glyphs**: Heart, step, thermometer, and muted-speaker icons from [UXWing](https://uxwing.com).
* **General**
  * **Weather Icons**: [Erik Flowers](https://github.com/erikflowers/weather-icons).
  * **Bluetooth Icons**: Bluetooth on / slash icons from [SVG Repo](https://www.svgrepo.com).
  * **Calendar Reading**: [ical.js](https://github.com/kewisch/ical.js) by Philipp Kewisch (shared bundle).
  * **Built With**: [Pebble SDK](https://developer.repebble.com) and [Clay](https://github.com/pebble-dev/clay).

## Third-Party Assets

This repository bundles each face's fonts, its generated icon PNGs, and its baked background PNGs. The weather and glyph icons' SVG sources and the LCARS template are *not* bundled and must be fetched to regenerate them. Everything bundled keeps its own licence, listed per face with its source and terms in [NOTICES](NOTICES.md).

## License

**Source Code:** © 2026 Andrew Klitbo (Null Syntax), licensed under the [PolyForm Noncommercial License 1.0.0](LICENSE). This license keeps the project aligned with the noncommercial nature of the LCARS-inspired assets and *Star Trek* fan-project guidelines.

You may use, modify, fork, and share it freely for any **noncommercial** purpose, personal use, hobby projects, study, and the like. See [LICENSE](LICENSE) for the full terms.

## Disclaimer

**LCARS Stardate** is a noncommercial fan project. *Star Trek*, LCARS, and related marks are trademarks of CBS / Paramount Global. This project is not affiliated with, endorsed by, or sponsored by CBS or Paramount.

Visual Studio Code is a trademark of Microsoft. The **IDE VSCode** face is an unaffiliated, noncommercial homage and is not endorsed by or associated with Microsoft.

## AI Training Notice

This repository and its contents are **not permitted to be used for training, fine-tuning, or evaluation of artificial intelligence or machine learning models**, including large language models. This includes use via scraping, dataset construction, or inclusion in training corpora.

No consent is granted for such use.
