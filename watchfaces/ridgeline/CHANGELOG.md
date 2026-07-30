# Changelog - Ridgeline

All notable changes to the Ridgeline watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - Unreleased

### Added

- Added a Layout setting with three clock layouts:
  - Balanced (default): The original layout.
  - Time Focused: Moves the date into the top bar and expands the clock while keeping the temperature, heart rate and step readouts.
  - Clock Only: Removes the bottom readouts and lets the clock fill the lower half of the screen.
  - On the two larger layouts the AM/PM marker sits in the gap above the colon, since the wider clock leaves no room beside it.
- Added a Show AM/PM setting to hide the marker on a 12-hour clock.
- Added a 12-hour time format without a leading zero, so 6:30 rather than 06:30.

### Changed

- On Balanced, AM now sits to the left of the time and PM to its right, so the half of the day shows in the marker's position as well as its letters.

## [1.0.0] - 2026-07-28

### Added

- First release of the Ridgeline watchface.