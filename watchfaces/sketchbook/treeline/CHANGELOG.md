# Changelog - Treeline

All notable changes to the Treeline watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- The watchface no longer tracks or stores health history that it never displays, eliminating unnecessary storage writes.
- The watchface no longer reads sleep, active minutes, or calories from the watch. It has never displayed them, and reading them required an unnecessary storage access once per minute.

### Fixed

- Fixed the watchface making the watch feel slow and unresponsive. It was building and storing a full history of heart rate and step activity that it never displays, and rebuilding it every minute delayed the watch's background work.
- Fixed the heart rate, step, and distance readings going stale while the watch was sitting still. They now refresh once per minute whether or not you have been moving.

## [1.1.0] - 2026-08-01

### Added

- Added support for the Round 2 (Gabbro).

### Changed

- The Quiet Time mark now shows whether Quiet Time is on or off, rather than appearing only while Quiet Time is active.

### Notes

On the Round 2:

- Round 2 support has been developed and tested in the emulator. I don't currently own a Round 2, so there may be hardware-specific issues that weren't possible to verify.
- Balanced is the only layout. The Layout setting is not shown.
- The bottom readouts are omitted, and the Tracks Readout setting is hidden.
- The temperature is shown on the cabin sign instead of below the date.
- The cabin sign is hidden when no weather reading is available.

## [1.0.1] - 2026-07-31

### Fixed

- Fixed the shadowed part of the moon showing as a blue patch rather than the night sky behind it.

## [1.0.0] - 2026-07-30

### Added

- First release of the Treeline watchface.
