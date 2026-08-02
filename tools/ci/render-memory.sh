#!/usr/bin/env bash
# Render every face's memory rows as one table on the run's summary page.
#
# The build is a matrix, one job per face, and GITHUB_STEP_SUMMARY is per job — so writing the
# table from inside the matrix would scatter one-line tables across seven jobs and make the only
# interesting question, which face is closest to the edge, impossible to answer at a glance.
# Instead each job leaves a .tsv behind and this collects them into one sorted table.
#
# Sorted by whichever ceiling a row is closest to, since that is the one that fails the build
# first: the 64 KB app image, or the 65535 uint16 virtual_size. For a face it is usually the latter.
#
# Reporting only, and it does not mind a missing face: one that failed to build has bigger problems
# than its heap, and the ones that did build are still worth seeing.
#
# Usage: tools/ci/render-memory.sh <dir-of-tsvs>

set -euo pipefail

die() {
  echo "$1" >&2
  exit 1
}

dir="${1:-}"
[[ -n "$dir" && -d "$dir" ]] || die "usage: tools/ci/render-memory.sh <dir-of-tsvs>"

rows=$(find "$dir" -name '*.tsv' -exec cat {} + 2>/dev/null | grep -v '^[[:space:]]*$' || true)
[[ -n "$rows" ]] || die "Error: no memory rows under $dir. Did the build jobs upload their artifacts?"

report=$(printf '%s\n' "$rows" | awk -F'\t' '
  BEGIN {
    image_limit   = 65536   # inject_metadata.py MAX_APP_BINARY_SIZE, not the platform config
    virtual_limit = 65535   # PebbleProcessInfo.virtual_size is a uint16_t
  }
  {
    face = $1; target = $2; platform = $3
    image = $4 + 0; virtual = $5 + 0; res = $6 + 0; ram = $7 + 0; free = $8 + 0

    image_pct   = (image * 100) / image_limit
    virtual_pct = (virtual * 100) / virtual_limit
    worst = (virtual_pct > image_pct) ? virtual_pct : image_pct

    mark = ""
    if (worst >= 90)      { mark = " :rotating_light:" }
    else if (worst >= 80) { mark = " :warning:" }

    # worst first, so the sort key leads
    printf "%09.4f\t| **%s** | `%s` | `%s` | %.1f KB (%d%%) | %.1f KB (%d%%)%s | %.0f KB | %.0f KB |\n", \
      worst, face, target, platform, image / 1024, image_pct, virtual / 1024, virtual_pct, mark, \
      free / 1024, res / 1024
  }
' | sort -rn | cut -f2- | awk '
  BEGIN {
    print "## Watchface Memory\n"
    print "| Face | Target | Platform | App Image | Static | Free Heap | Resources |"
    print "| :--- | :--- | :--- | ---: | ---: | ---: | ---: |"
  }
  { print }
  END {
    print ""
    print "**App Image** is `load_size` plus the relocation table, capped at 64 KB by"
    print "`MAX_APP_BINARY_SIZE = 0x10000` in inject_metadata.py, which ignores the platform"
    print "config and so applies to emery and gabbro too."
    print ""
    print "**Static** is `virtual_size` (.text + .data + .bss), capped at 65535 because that"
    print "header field is a uint16_t. For a face this is normally the tighter of the two."
    print ""
    print "**Free Heap** is the RAM left from the 128 KB after the app, system allocations"
    print "and resources. The mark tracks whichever ceiling the row is closest to."
  }
')

printf '%s\n' "$report"

if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
  printf '%s\n' "$report" >> "$GITHUB_STEP_SUMMARY"
fi
