#!/usr/bin/env bash
# Render every face's memory rows as one table on the run's summary page.
#
# The build is a matrix, one job per face, and GITHUB_STEP_SUMMARY is per job — so writing the
# table from inside the matrix would scatter one-line tables across seven jobs and make the only
# interesting question, which face is closest to the edge, impossible to answer at a glance.
# Instead each job leaves a .tsv behind and this collects them into one sorted table.
#
# Sorted by how full the app image is, because that is the ceiling that actually fails a build:
# inject_metadata.py enforces 64 KB regardless of what the platform config says emery allows.
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

report=$(printf '%s\n' "$rows" | sort -t$'\t' -k3,3nr | awk -F'\t' '
  BEGIN {
    limit = 65536   # inject_metadata.py MAX_APP_BINARY_SIZE, not the platform config
    print "## Watchface Memory\n"
    print "| Face | Target | App Image | of 64 KB | Free Heap | Resources |"
    print "| :--- | :--- | ---: | ---: | ---: | ---: |"
  }
  {
    face = $1; target = $2; image = $3 + 0; res = $4 + 0; ram = $5 + 0; free = $6 + 0
    pct = (image * 100) / limit

    # the image is the one that stops a build, so it is the one that gets marked. anything past
    # four fifths is worth knowing about before the release that tips it over
    mark = ""
    if (pct >= 90)      { mark = " :rotating_light:" }
    else if (pct >= 80) { mark = " :warning:" }

    printf "| **%s** | `%s` | %.1f KB%s | %d%% | %.0f KB | %.0f KB |\n", \
      face, target, image / 1024, mark, pct, free / 1024, res / 1024
  }
  END {
    print ""
    print "**App Image** is the build limit. Pebble enforces a 64 KB maximum"
    print "(`MAX_APP_BINARY_SIZE = 0x10000`), even though emery lists 128 KB of app storage."
    print ""
    print "**Free Heap** is the RAM left over from the 128 KB on the watch after the app,"
    print "system allocations and resources."
  }
')

printf '%s\n' "$report"

if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
  printf '%s\n' "$report" >> "$GITHUB_STEP_SUMMARY"
fi
