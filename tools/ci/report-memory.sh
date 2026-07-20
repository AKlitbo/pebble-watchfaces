#!/usr/bin/env bash
# Measure what actually stops a face building, per target, into a machine-readable row.
#
# There are two ceilings and only one of them bites:
#
#   * the app image, capped at 64 KB. pebble_sdk_platform.py claims 128 KB for emery, but
#     inject_metadata.py carries its own MAX_APP_BINARY_SIZE = 0x10000 at module scope and nothing
#     overrides it from the platform config, so the build raises "App image size is N ... Must be
#     smaller than 65536 bytes" whatever the platform says. This is the one that fails a build.
#   * the heap, out of 128 KB of RAM. Worth watching, but nothing here is close to it.
#
# So the image is the headline and the heap rides along. `pebble build` prints the heap figures
# during the link (and only then: an incremental build with nothing to relink skips the report
# entirely, which is why this wants a clean build. CI always is). The image size is not in the log
# at all, so it comes off the built binary.
#
# Reporting only. It never fails a build for being large, because the SDK already does that at the
# only threshold that matters, and a softer limit here would just get raised whenever it fired.
#
# Usage: tools/ci/report-memory.sh <build-log> <face> [out.tsv]

set -euo pipefail

die() {
  echo "$1" >&2
  exit 1
}

log="${1:-}"
face="${2:-}"
out="${3:-}"

[[ -n "$log" && -n "$face" ]] || die "usage: tools/ci/report-memory.sh <build-log> <face> [out.tsv]"
[[ -f "$log" ]] || die "Error: No build log at $log. Run 'bash build.sh <face> | tee $log' first."

grep -q "APP MEMORY USAGE" "$log" \
  || die "Error: No memory report in $log. Either the build did not link, or it was incremental and had nothing to relink (try --clean)."

# each block's target only appears on the "Leaving directory .../targets/<target>/build" line after
# it, so the numbers are held until the name arrives. the counts sit immediately before the literal
# "bytes", which is steadier than counting in from either end of the line
targets=$(awk '
  function value(   i, n, f) {
    n = split($0, f, /[ \t]+/)
    for (i = 1; i <= n; i++) { if (f[i] == "bytes") { return f[i - 1] + 0 } }
    return -1
  }
  /APP MEMORY USAGE/             { inblock = 1; next }
  inblock && /Total size of res/ { res  = value(); next }
  inblock && /Total footprint/   { ram  = value(); next }
  inblock && /Free RAM avail/    { free = value(); inblock = 0; pending = 1; next }
  pending && /Leaving directory/ {
    target = $0
    sub(/.*\/targets\//, "", target)
    sub(/\/build.*/, "", target)
    if (res >= 0 && ram >= 0 && free >= 0) { printf "%s\t%d\t%d\t%d\n", target, res, ram, free; n++ }
    pending = 0
  }
  END { if (n == 0) { exit 1 } }
' "$log") || die "Error: found a memory report in $log but could not read its numbers."

rows=""
while IFS=$'\t' read -r target res ram free; do
  [[ -n "$target" ]] || continue

  # the image the 64 KB check measures. it is not in the log, so read it off the binary the build
  # just wrote. a missing one is reported as 0 rather than skipping the target, so the row still
  # shows and the gap is obvious
  image=0
  bin="targets/$target/build/emery/pebble-app.bin"
  [[ -f "$bin" ]] && image=$(stat -c %s "$bin")

  rows+="${face}	${target}	${image}	${res}	${ram}	${free}"$'\n'
done <<< "$targets"

printf '%s' "$rows"

if [[ -n "$out" ]]; then
  printf '%s' "$rows" > "$out"
fi
