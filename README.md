# Pebble Watchfaces

A collection of watchfaces for the Pebble Time 2 (**Emery**), built on one shared engine. Each face
shows the time, date, weather, heart rate, steps, and battery, wrapped in its own interface, with
themes selectable from a Clay settings page.

| Watchface | Preview |
| :--- | :--- |
| **LCARS Stardate**<br>[changelog](watchfaces/lcars-stardate/CHANGELOG.md) | <img src=".github/images/lcars-stardate/theme_classic.png" width="100" title="Classic"> <img src=".github/images/lcars-stardate/theme_nemesis-blue.png" width="100" title="Nemesis Blue"> <img src=".github/images/lcars-stardate/theme_mono.png" width="100" title="Classic Mono"> <img src=".github/images/lcars-stardate/theme_lower-decks.png" width="100" title="Lower Decks"> <img src=".github/images/lcars-stardate/theme_lower-decks-mono.png" width="100" title="Lower Decks Mono"> <img src=".github/images/lcars-stardate/theme_lower-decks-padd.png" width="100" title="Lower Decks PADD"> <img src=".github/images/lcars-stardate/theme_lower-decks-padd-mono.png" width="100" title="Lower Decks PADD Mono"> |
| **Radar Array**<br>[changelog](watchfaces/radar-array/CHANGELOG.md) | <img src=".github/images/radar-array/theme_default.png" width="100" title="Default"> <img src=".github/images/radar-array/theme_crimson.png" width="100" title="Crimson"> <img src=".github/images/radar-array/theme_neon.png" width="100" title="Neon"> <img src=".github/images/radar-array/theme_phosphor.png" width="100" title="Phosphor"> <img src=".github/images/radar-array/theme_rescue.png" width="100" title="Rescue"> <img src=".github/images/radar-array/theme_stealth.png" width="100" title="Stealth"> <img src=".github/images/radar-array/theme_mono.png" width="100" title="Mono"> |
| **IDE VSCode**<br>[changelog](watchfaces/ide-vscode/CHANGELOG.md) | <img src=".github/images/ide-vscode/theme_dark.png" width="100" title="Dark"> <img src=".github/images/ide-vscode/theme_light.png" width="100" title="Light"> <img src=".github/images/ide-vscode/theme_terminal.png" width="100" title="Terminal"> <img src=".github/images/ide-vscode/theme_cyberpunk.png" width="100" title="Cyberpunk"> <img src=".github/images/ide-vscode/theme_synthwave.png" width="100" title="Synthwave"> <img src=".github/images/ide-vscode/theme_mono.png" width="100" title="Mono"> |

## Install

Download a face's `.pbw` from [Releases](https://github.com/AKlitbo/pebble-watchfaces/releases) and
open it with the Pebble app on your phone.

Faces version independently, so releases are tagged per face as `<face>-v<version>` and each carries
one `.pbw`. Release notes are that version's `CHANGELOG.md` entry. A face built for a single watch
names its platform in the asset, so `lcars-stardate-emery-1.5.0.pbw` is Emery only.

## Project Structure

The shared engine lives at the root. Each face owns only what makes it that face.

* **`watchfaces/<face>/`**: one watchface, self-contained.
  * **`src/c/`**: device code: the zone layout, readouts, baked-frame theme, widgets, settings, dev harness.
  * **`src/pkjs/`**: the Clay config page and the phone-side settings/weather bridge (a thin wrapper over `lib/ts`).
  * **`config/pebble.appinfo.json`**: the face's identity: uuid, version, message keys, and its whole resource list.
  * **`CHANGELOG.md`**: that face's release history. Faces version independently.
  * **`frame/`**: the HTML/CSS chrome the background bitmaps are baked from, plus `frame.config.json`.
  * **`resources/`**: bundled fonts, baked background PNGs, and rendered icon PNGs.
* **`lib/`**: the reusable base every face shares (`c/` device engine, `ts/` PebbleKit JS, `py/` waf helpers, `tools/` generators, `css/` the Pebble-64 gamut every frame links).
* **`config/`**: shared tooling config (tsconfigs, eslint, vitest).
* **`tools/`**: build and dev tooling (icon rasterizer, manifest generator, pkjs build, frame baker, waf template, CI scripts).
* **`targets/<face>/`**: the build sandbox waf runs in, one per face. Entirely generated and gitignored. Its `package.json`, `wscript`, and `emit/` are all produced by the build.
* **`vendor/`**: third-party source SVGs and the LCARS template (gitignored, see [Third-Party Assets](#third-party-assets)).
* **`build.sh`**: regenerates a face's manifest, compiles its TypeScript pkjs, and runs `pebble build`.

Anything with a `.g.` in the name is generated and should not be hand-edited: rerun the matching
`npm run gen:*`. CI checks that the committed output still matches.

### Adding a Face

Create `watchfaces/<name>/` with the layout above, then build it. The sandbox, manifest, and waf
entry point are all generated from the face's name and appinfo. Add `<name>` to the CI matrix in
[.github/workflows/ci.yml](.github/workflows/ci.yml) so it builds on every push.

## Releasing

Pushing a `<face>-v<version>` tag is the whole process. [release.yml](.github/workflows/release.yml)
builds that face, takes its notes from the matching `CHANGELOG.md` section, and publishes the `.pbw`.

```sh
# date the [1.5.0] heading in watchfaces/lcars-stardate/CHANGELOG.md first, then
git tag lcars-stardate-v1.5.0
git push origin lcars-stardate-v1.5.0
```

The tag version must match `version` in that face's `config/pebble.appinfo.json`, the changelog entry
must be dated, and the tag must not already be released. The workflow checks all three before it
spends time on a build, so a mistake costs seconds.

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

Three providers, selectable in Settings:

- **Open-Meteo** *(recommended)*: free, no account or API key.
- **WeatherAPI**: free tier, needs an account and API key.
- **OpenWeatherMap**: free tier, needs an account and API key.

All three cover the basics: temperature, conditions, wind, humidity, pressure, feels like, and sunrise/sunset. OpenWeatherMap's free tier leaves out UV index, dew point, today's high/low, and chance of rain, so those are backfilled from Open-Meteo.

## Calendar (iCal)

On a face that supports it, calendar comes from a private iCal (`.ics`) feed, so any service that publishes one works (Google Calendar's secret address is the usual choice). Paste the feed URL into Settings and upcoming events are fetched automatically.

Only event time, title, and location are sent to the watch, and events more than a week away are ignored.

Feeds are parsed with [ical.js](https://github.com/kewisch/ical.js), including recurring events, exceptions, cancellations, and time zones. It is licensed under MPL 2.0 and included as a separate file. See [NOTICES](NOTICES.md).

Alarms and attendees are currently ignored.

## Stock Providers

On a face that supports it, four providers are selectable in Settings. All are free, and all except Yahoo need an account and API key. Set your ticker(s) and key there too.

- **Finnhub** *(recommended)*: real-time US quotes, follows your refresh interval.
- **Yahoo**: real-time, no API key, and the widest coverage (US and international stocks, ETFs, indices, and crypto). Unofficial feed, so it can break without notice.
- **Twelve Data**: global markets. Follows your interval while markets are open, no faster than every 15 minutes, then slows down after hours to stay within daily limits.
- **Alpha Vantage**: one end-of-day quote after the close, with a small daily request allowance.

All four return the same core quote data: last price, price change, percentage change, and latest trading day.

---

## Credits

* **LCARS Stardate**
  * **LCARS Design**: LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications.
  * **Typography**: [Antonio](https://fonts.google.com/specimen/Antonio).
  * **Glyphs**: Heart, step, and thermometer icons from [UXWing](https://uxwing.com).
* **Radar Array**
  * **Typography**: [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono).
* **IDE VSCode**
  * **Typography**: [Teko](https://fonts.google.com/specimen/Teko) and [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono).
* **General**
  * **Weather Icons**: [Erik Flowers](https://github.com/erikflowers/weather-icons).
  * **Bluetooth Icons**: Bluetooth on / slash icons from [SVG Repo](https://www.svgrepo.com).
  * **Calendar Reading**: [ical.js](https://github.com/kewisch/ical.js) by Philipp Kewisch (shared bundle).
  * **Built With**: [Pebble SDK](https://developer.repebble.com) and [Clay](https://github.com/pebble-dev/clay).

## Third-Party Assets

This repository bundles each face's fonts, its generated icon PNGs, and its baked background PNGs.
The weather and glyph icons' SVG sources and the LCARS template are *not* bundled and must be
fetched to regenerate them. Everything bundled keeps its own licence, listed per face with its
source and terms in [NOTICES](NOTICES.md).

## License

**Source Code:** © 2026 Andrew Klitbo (Null Syntax), licensed under the [PolyForm Noncommercial License 1.0.0](LICENSE).
This license keeps the project aligned with the noncommercial nature of the LCARS-inspired assets
and *Star Trek* fan-project guidelines.

You may use, modify, fork, and share it freely for any **noncommercial** purpose, personal use,
hobby projects, study, and the like. See [LICENSE](LICENSE) for the full terms.

## Disclaimer

**LCARS Stardate** is a noncommercial fan project. *Star Trek*, LCARS, and related marks are
trademarks of CBS / Paramount Global. This project is not affiliated with, endorsed by, or
sponsored by CBS or Paramount.

Visual Studio Code is a trademark of Microsoft. The **IDE VSCode** face is an unaffiliated,
noncommercial homage and is not endorsed by or associated with Microsoft.

## AI Training Notice

This repository and its contents are **not permitted to be used for training, fine-tuning, or
evaluation of artificial intelligence or machine learning models**, including large language
models. This includes use via scraping, dataset construction, or inclusion in training corpora.

No consent is granted for such use.
