# Third-party notices

The watchfaces in this repository are under the PolyForm Noncommercial License, see [LICENSE](LICENSE).
They bundle the third-party work below, each of which keeps its own licence. This file is how those
licences are passed on to anyone a watchface or this repository is given to.

For who made these and what they are used for, see the Credits section of the [README](README.md).

Each face bundles only what it uses, so the notices are grouped by what ships in which `.pbw`. The
Code and General sections apply to every face.

## Code

### ical.js

The phone-side calendar reader in the shared PebbleKit JS bundle. None of these faces surface
calendar panels, but the shared bundle carries the reader, so it ships in every built `.pbw`.

- **Source:** <https://github.com/kewisch/ical.js>
- **Licence:** Mozilla Public License 2.0, published at <https://www.mozilla.org/en-US/MPL/2.0/>.
  The full text also ships in the package at `node_modules/ical.js/LICENSE`
- **What ships:** the package's own prebuilt `dist/ical.es5.min.cjs`, copied in unchanged by the
  pkjs build. It is not modified, patched or re-bundled, so the source that produced it is the
  upstream repository above, and its licence header travels inside the bundle

The MPL covers ical.js and nothing else here. Section 1.10 of that licence puts it plainly: a file
carrying none of its code is not a modification of it, so the rest of this project stays under the
PolyForm terms. Keeping ical.js in a file of its own rather than mixing it into ours is deliberate,
and is what section 1.7 asks of a larger work.

## Fonts

Each face bundles its `.ttf` files under `watchfaces/<face>/resources/fonts/`, and the build converts
them into the watch's own font format, so both this repository and the built watchface carry them.

All fonts below are under the SIL Open Font License 1.1, published at <https://openfontlicense.org>.
Each face ships an `OFL.txt` beside its fonts carrying the full licence text and the copyright
notice each font declares in its own metadata.

### LCARS Stardate

- **[Antonio](https://fonts.google.com/specimen/Antonio)**: Copyright The Antonio Project Authors,
  with Reserved Font Name "Antonio". Licence text in
  [OFL.txt](watchfaces/lcars-stardate/resources/fonts/OFL.txt)

### Radar Array

- **[Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono)**: Copyright (c) 2012,
  Carrois Type Design, Ralph du Carrois, with Reserved Font Name 'Share'. Licence text in
  [OFL.txt](watchfaces/radar-array/resources/fonts/OFL.txt)

### IDE VSCode

Both fonts share one [OFL.txt](watchfaces/ide-vscode/resources/fonts/OFL.txt), which carries a
copyright notice for each.

- **[Teko](https://fonts.google.com/specimen/Teko)**: Copyright 2023 The Teko Project Authors
- **[Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono)**: Copyright (c) 2012,
  Carrois Type Design, Ralph du Carrois, with Reserved Font Name 'Share'

## Icons

The SVG sources for the weather and glyph icons are not in this repository. They are fetched to
regenerate the icons, and only the rendered PNGs are bundled. The bluetooth SVGs are the exception,
bundled under `vendor/svgrepo/`.

Which of these a face bundles depends on what it draws. See each face's `resources/icons.json`.
Radar Array draws no weather icons, so it bundles only the bluetooth glyphs.

- **[Weather Icons by Erik Flowers](https://github.com/erikflowers/weather-icons)**: SIL Open Font
  License 1.1 for the font, MIT for the code. Rendered PNGs bundled by LCARS Stardate and IDE / VS Code
- **[UXWing](https://uxwing.com)** (heart, step, thermometer glyphs): the
  [UXWing licence](https://uxwing.com/license/), which allows use without attribution but does not
  allow redistributing the icons themselves. SVG sources fetched separately, rendered PNGs bundled
  by LCARS Stardate
- **[SVG Repo](https://www.svgrepo.com)** (the bluetooth glyphs):
  [CC Attribution](https://www.svgrepo.com/page/licensing/#CC%20Attribution). SVG sources bundled
  under `vendor/svgrepo/`, rendered PNGs bundled by every face

## Template

- **LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com)**: the HTML/CSS the
  LCARS Stardate frame backgrounds are baked from, used with modifications. Fetched separately and
  not bundled. Please visit the site to download and support the creator.

## Trademarks

*Star Trek*, LCARS, and related marks are trademarks of CBS / Paramount Global. Visual Studio Code is
a trademark of Microsoft. These faces are unaffiliated, noncommercial homages and are not endorsed by
or associated with those rights holders. No trademark licence is granted or implied.

Refer to each source above for the full terms.
