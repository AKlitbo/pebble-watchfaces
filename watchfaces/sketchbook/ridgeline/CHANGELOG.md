# Changelog - Ridgeline

All notable changes to the Ridgeline watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.3.0] - 2026-08-03

### Changed

- The watchface no longer tracks or stores health history that it never displays, eliminating unnecessary storage writes.
- The watchface no longer reads sleep, active minutes, or calories from the watch. It has never displayed them, and reading them required an unnecessary storage access once per minute.

### Fixed

- Fixed the watchface making the watch feel slow and unresponsive. It was building and storing a full history of heart rate and step activity that it never displays, and rebuilding it every minute delayed the watch's background work.
- Fixed the heart rate, step, and distance readings going stale while the watch was sitting still. They now refresh once per minute whether or not you have been moving.

## [1.2.0] - 2026-08-01

### Added

- Added support for the Round 2 (Gabbro).

### Changed

- The Quiet Time mark now shows whether Quiet Time is on or off, rather than appearing only while Quiet Time is active.

### Notes

On the Round 2:

- Round 2 support has been developed and tested in the emulator. I don't currently own a Round 2, so there may be hardware-specific issues that weren't possible to verify.
- Balanced is the only layout. The Layout setting is not shown.
- The bottom readouts are omitted, and the Tracks Readout setting is hidden.
- The temperature is shown on the trail sign instead of below the date.
- The trail sign is hidden when no weather reading is available.

## [1.1.2] - 2026-07-31

### Fixed

- Fixed the shadowed part of the moon showing as a blue patch rather than the night sky behind it.

## [1.1.1] - 2026-07-30

### Fixed

- Fixed the clock changing size on Time Focused and Clock Only. Wider times could overflow the available space and shrink the clock while you were looking at it. The clock is now sized so every supported time fits without resizing.
- Fixed the clock sitting slightly high on Clock Only. It is now centred in the clear space below the mountains.
- Fixed the AM/PM marker overlapping the ridgeline on Balanced.

## [1.1.0] - 2026-07-29

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