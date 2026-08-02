#!/usr/bin/env bash
# Measure what actually stops a face building, per target, into a machine-readable row.
#
# Three ceilings, and the heap is not the one to watch:
#
#   * app image (load_size + reloc table), 64 KB. inject_metadata.py hardcodes
#     MAX_APP_BINARY_SIZE = 0x10000 and ignores the platform config, so emery and gabbro get 64 KB
#     despite declaring 128 KB.
#   * virtual_size (.text + .data + .bss), 65535, because that header field is a uint16_t.
#     Usually the tighter of the two, since a face has little relocation table. Watch this one.
#   * heap, out of 128 KB. Nothing here is close to it.
#
# The heap figures are printed during the link, and only then, so this wants a clean build (CI
# always is). Neither size is in the log; both come off the binary's PebbleProcessInfo header.
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

# the target name only arrives on the "Leaving directory .../targets/<target>/build" line, one per
# sandbox, so blocks are buffered and flushed there — a target building two platforms has two
# blocks but one such line. the platform comes off the block header ("EMERY APP MEMORY USAGE").
# counts sit immediately before the literal "bytes", steadier than counting in from either end
targets=$(awk '
  function value(   i, n, f) {
    n = split($0, f, /[ \t]+/)
    for (i = 1; i <= n; i++) { if (f[i] == "bytes") { return f[i - 1] + 0 } }
    return -1
  }
  /APP MEMORY USAGE/             { inblock = 1; plat[++p] = tolower($1); next }
  inblock && /Total size of res/ { res[p]  = value(); next }
  inblock && /Total footprint/   { ram[p]  = value(); next }
  inblock && /Free RAM avail/    { free[p] = value(); inblock = 0; next }
  /Leaving directory/ {
    if (p == 0) { next }
    target = $0
    sub(/.*\/targets\//, "", target)
    sub(/\/build.*/, "", target)
    for (i = 1; i <= p; i++) {
      if (res[i] >= 0 && ram[i] >= 0 && free[i] >= 0) {
        printf "%s\t%s\t%d\t%d\t%d\n", target, plat[i], res[i], ram[i], free[i]; n++
      }
    }
    p = 0
    delete plat; delete res; delete ram; delete free
  }
  END { if (n == 0) { exit 1 } }
' "$log") || die "Error: found a memory report in $log but could not read its numbers."

rows=""
while IFS=$'\t' read -r target platform res ram free; do
  [[ -n "$target" ]] || continue

  # image is the file size; virtual_size is bytes 128..129 of PebbleProcessInfo. a missing binary
  # reports 0 rather than skipping the target, so the row still shows and the gap is obvious
  image=0
  virtual=0
  bin="targets/$target/build/$platform/pebble-app.bin"
  if [[ -f "$bin" ]]; then
    image=$(stat -c %s "$bin")
    virtual=$(python3 -c 'import struct,sys; print(struct.unpack_from("<H", open(sys.argv[1],"rb").read(130), 0x80)[0])' "$bin" 2>/dev/null || echo 0)
  fi

  rows+="${face}	${target}	${platform}	${image}	${virtual}	${res}	${ram}	${free}"$'\n'
done <<< "$targets"

printf '%s' "$rows"

if [[ -n "$out" ]]; then
  printf '%s' "$rows" > "$out"
fi
