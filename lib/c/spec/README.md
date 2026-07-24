# C Unit Tests

Host-side tests for the maths in `lib/c/core`: dates, moon phase, beats, number and text formatting, unit conversion. They build with a plain host `gcc`, no Pebble SDK, and run off the watch.

## Running

From WSL or any Linux with `gcc` and `make`:

```sh
cd lib/c/spec
make
```

`make` builds every `*.spec.c` under `lib/c` as a standalone [Unity](https://github.com/ThrowTheSwitch/Unity) program against the code it covers, runs it, and returns non-zero on any failure. It takes about a second and doubles as the CI gate. `make clean` drops `build/`.

Specs sit next to their source (like `*.spec.ts`) and never reach the watch: `lib/py/waf_helpers.py` drops `**/*.spec.c` and `spec/` from the firmware glob. Adding one needs no Makefile entry: drop `foo.spec.c` beside `foo.c` and it is found, built and run.

## What is not here

Only `lib/c/core` is covered, and that is on purpose. It is plain arithmetic that needs nothing but a compiler, so a test is cheap and a wrong answer is invisible in review and obvious on the wrist. A one degree error in the moon maths or an off-by-one in the ISO week reads as a bug on someone's watch weeks later.

`lib/c/pebble` is a different animal. It talks to the SDK, so testing it off the watch means standing in for the SDK: persist, timers, the clock, dictionaries, the AppMessage transport. That fake has to mirror the real thing byte for byte to be worth anything, and when it drifts the tests still pass while the watch is wrong. The trade is a lot of scaffolding to test code that mostly forwards to Pebble, and Pebble is not ours to test. It is left to the device.

## Unity (Third-Party)

`unity/` is a vendored copy of the [Unity](https://github.com/ThrowTheSwitch/Unity) test framework, kept verbatim. Each file carries its own MIT copyright header.

* Source: [ThrowTheSwitch/Unity](https://github.com/ThrowTheSwitch/Unity), the three `src/` files `unity.c`, `unity.h`, and `unity_internals.h`.
* Pinned to [v2.6.1](https://github.com/ThrowTheSwitch/Unity/releases/tag/v2.6.1), commit `cbcd08fa7de711053a3deec6339ee89cad5d2697`.
* License: MIT.

To update, re-pull the three files from a release tag (never `master`), then bump the tag and commit above:

```sh
tag=v2.6.1
for f in unity.c unity.h unity_internals.h; do
  curl -fsSL -o "unity/$f" "https://raw.githubusercontent.com/ThrowTheSwitch/Unity/$tag/src/$f"
done
```
