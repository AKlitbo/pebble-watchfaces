#!/usr/bin/env bash
#
# tap-walk.sh: screenshot every UNIQUE state of the dev tap-walk.
#
# With a DEV_TAP_WALK_* switch on (see watchfaces/<face>/src/c/dev/dev.h) each accel tap steps the face
# to the next module/appearance. This loops screenshot -> emu-tap -> screenshot and
# stops once a shot matches the very first one (the walk has wrapped back to start).
#
# Some taps render identically to a shot we already have (e.g. on a standard panel
# the borderless / no-header / data-only passes all look the same), so we dedupe as
# we go: a shot is only saved when its sha256 is one we haven't seen yet.
#
# Because DEV_FORCE_TIME pins the clock, the same state renders byte-identical, so
# sha256 is a reliable identity + "back to start" signal.
#
# Run from WSL, from the repo root:
#   tools/dev/tap-walk.sh --target=gridlock-face
#
# The target is the build sandbox name, which is the face name for most faces and the
# per-target name for a face that ships several (gridlock-face, gridlock-app).
#
# Assumes the target is already built and installed on the emery emulator. Pass
# --install to build + install first, which needs --face when the two names differ.

set -euo pipefail

EMULATOR="emery"
TARGET=""           # the sandbox under targets/, e.g. ridgeline or gridlock-face
FACE=""             # the source face for build.sh, when it differs from the target
OUT_DIR=".tmp/tap-walk-shots"   # scratch output, gitignored via .tmp
SETTLE=0.9          # seconds to let the firmware redraw after a tap
MAX_TAPS=200        # hard stop so we never loop forever
DO_INSTALL=0

for arg in "$@"; do
    case "$arg" in
        --install) DO_INSTALL=1 ;;
        --emulator=*) EMULATOR="${arg#*=}" ;;
        --out=*) OUT_DIR="${arg#*=}" ;;
        --target=*) TARGET="${arg#*=}" ;;
        --face=*) FACE="${arg#*=}" ;;
        -h|--help)
            grep '^#' "$0" | sed 's/^# \?//'
            exit 0
            ;;
        *) echo "unknown arg: $arg" >&2; exit 1 ;;
    esac
done

if [[ -z "$TARGET" ]]; then
    echo "no target given. pass --target=<sandbox>, e.g. --target=ridgeline or --target=gridlock-face" >&2
    exit 1
fi

PBW="targets/$TARGET/build/$TARGET.pbw"

if [[ "$DO_INSTALL" == "1" ]]; then
    # a face that ships several targets builds them all under its own name, so build.sh wants the
    # face while the .pbw is named after the target
    echo ">> building + installing $TARGET on $EMULATOR"
    ./build.sh "${FACE:-$TARGET}"
    pebble install --emulator "$EMULATOR" "$PBW"
    sleep 2
fi

if [[ ! -f "$PBW" ]]; then
    echo "no build at $PBW. build it first, or pass --install" >&2
    exit 1
fi

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

CANDIDATE="$OUT_DIR/.candidate.png"

# grab a screenshot into the candidate file and echo its sha256
grab() {
    pebble screenshot --no-open --emulator "$EMULATOR" "$CANDIDATE" >/dev/null 2>&1
    sha256sum "$CANDIDATE" | cut -d' ' -f1
}

declare -A seen        # sha256 -> 1 for every state we've already saved
unique=0

echo ">> capturing starting state"
first_hash="$(grab)"
mv -f "$CANDIDATE" "$(printf '%s/shot_%03d.png' "$OUT_DIR" "$unique")"
seen["$first_hash"]=1
printf '   shot_%03d  %s  (start)\n' "$unique" "$first_hash"

for tap in $(seq 1 "$MAX_TAPS"); do
    pebble emu-tap --emulator "$EMULATOR" >/dev/null 2>&1
    sleep "$SETTLE"

    hash="$(grab)"

    if [[ "$hash" == "$first_hash" ]]; then
        # wrapped back to the start, we're done
        rm -f "$CANDIDATE"
        echo ">> wrapped back to start after $tap taps"
        echo ">> $unique unique states captured in $OUT_DIR/"
        exit 0
    fi

    if [[ -n "${seen[$hash]:-}" ]]; then
        # a tap that renders the same as a state we already have, skip it
        rm -f "$CANDIDATE"
        printf '   tap %-3d   %s  (dup, skipped)\n' "$tap" "$hash"
        continue
    fi

    unique=$((unique + 1))
    seen["$hash"]=1
    mv -f "$CANDIDATE" "$(printf '%s/shot_%03d.png' "$OUT_DIR" "$unique")"
    printf '   shot_%03d  %s\n' "$unique" "$hash"
done

echo ">> hit MAX_TAPS ($MAX_TAPS) without wrapping, check DEV_TAP_WALK_MODULES is on" >&2
echo ">> $unique unique states captured in $OUT_DIR/" >&2
exit 1
