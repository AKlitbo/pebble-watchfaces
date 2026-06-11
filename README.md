# Stardate Watchfaces

A collection of LCARS (*Star Trek*) themed watchfaces for Pebble smartwatches. These watchfaces display the time, date, weather, heart rate, step count, and battery status, all wrapped in a Starfleet-inspired LCARS interface.

## Project Structure

This repository contains multiple watchface variants and the shared library that powers them:

* **`watchfaces/`**: Individual watchface projects.
  * `stardate-emery`: Optimized for the Pebble Time 2 (**Emery**).
  * `stardate-gabbro`: Optimized for the Pebble Round 2 (**Gabbro**).
* **`shared/`**: Common C and JavaScript code used across all watchfaces, including weather providers, unit conversion, health data, and shell components.
* **`tools/`**: Development utilities for generating frame bitmaps and rasterizing icons from SVG sources.
* **`vendor/`**: Third-party assets and templates.

## Credits

* **LCARS Design**: LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications.
* **Typography**: [Antonio](https://fonts.google.com/specimen/Antonio).
* **Weather Icons**: [Erik Flowers](https://github.com/erikflowers/weather-icons).
* **Glyphs**: Heart, step, and thermometer icons from [UXWing](https://uxwing.com).
* **Built With**: [Pebble SDK](https://developer.repebble.com) and [Clay](https://github.com/pebble-dev/clay).

## License

**Source Code:** © 2026 Andrew Klitbo, licensed under the [PolyForm Noncommercial License 1.0.0](LICENSE). This license was chosen to help keep the project aligned with the non-commercial nature of the LCARS-inspired assets and Star Trek fan-project guidelines.

You may use, modify, fork, and share it freely for any **noncommercial** purpose, personal use, hobby projects, study, and the like. See [LICENSE](LICENSE) for the full terms.

## Disclaimer

This is a non-commercial fan project. *Star Trek*, LCARS, and related marks are trademarks of CBS / Paramount Global. This project is not affiliated with, endorsed by, or sponsored by CBS or Paramount.

## AI Training Notice

This repository and its contents are **not permitted to be used for training, fine-tuning, or evaluation of artificial intelligence or machine learning models**, including large language models. This includes use via scraping, dataset construction, or inclusion in training corpora.

No consent is granted for such use.

## Third-Party Assets

This repository bundles the Antonio font and generated icon PNGs under `shared/resources/icons/` (rasterized from vendored SVG sources). It does **not** bundle the raw vendor sources: the Weather Icons, UXWing SVGs, and the TheLCARS.com frame template are stored under `vendor/` and must be fetched independently to regenerate icons or bake frames. They remain under their respective open-source licenses or terms:

* [Antonio Font](https://fonts.google.com/specimen/Antonio) (SIL Open Font License 1.1), bundled
* [Weather Icons by Erik Flowers](https://github.com/erikflowers/weather-icons) (SIL Open Font License 1.1 / MIT), rasterized from SVG sources, rendered PNGs bundled
* [UXWing Icons](https://uxwing.com) (UXWing License), SVG sources fetched separately, rendered PNGs bundled
* LCARS Inspired Website Template by [TheLCARS.com](https://www.thelcars.com), with modifications, fetched separately, not bundled

Please refer to their original source repositories and websites for full license and usage details.