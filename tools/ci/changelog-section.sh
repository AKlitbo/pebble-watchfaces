#!/usr/bin/env bash
# Pull one version's section out of a face's CHANGELOG.md, for use as GitHub release notes.
#
# The changelogs are Keep a Changelog format, so every release is already written up under a
# "## [X.Y.Z] - YYYY-MM-DD" header. Release notes are that section verbatim - there is no second
# place to keep them in sync.
#
# Also the release gate: an entry still dated "Unreleased" means the changelog was never finished,
# so this exits non-zero rather than publishing a half-written release.
#
# Usage: tools/ci/changelog-section.sh <face> <version> [--date]
#   --date   print the section's release date instead of its body

set -euo pipefail

# helper to print errors to stderr and exit
die() {
  echo "$1" >&2
  exit 1
}

if [[ $# -lt 2 ]]; then
  die "Usage: $0 <face> <version> [--date]"
fi

face="$1"
version="$2"
mode="${3:-body}"

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
changelog="$here/watchfaces/$face/CHANGELOG.md"

[[ -f "$changelog" ]] || die "Error: No changelog at watchfaces/$face/CHANGELOG.md"

# "## [1.4.0] - 2026-07-03" -> everything after the " - "
header=$(grep -m1 -F "## [$version]" "$changelog" || true)

[[ -n "$header" ]] || die "Error: No [$version] section in watchfaces/$face/CHANGELOG.md"

release_date="${header#*] - }"

if [[ "$release_date" == "Unreleased" ]]; then
  die "Error: watchfaces/$face/CHANGELOG.md still has [$version] as Unreleased: date it before releasing"
fi

if [[ "$mode" == "--date" ]]; then
  printf '%s\n' "$release_date"
  exit 0
fi

# print between this version's header and the next "## [" one.
# this single Awk command extracts the section AND trims leading/trailing blank lines.
body=$(awk -v header="## [$version]" '
  index($0, header) == 1 { collecting = 1; next }
  collecting && /^## \[/ { exit }
  collecting {
    if (!NF) {
      held++
    } else {
      if (started) {
        for (i = 0; i < held; i++) print ""
      }
      started = 1
      held = 0
      print
    }
  }
' "$changelog")

if [[ -z "$body" ]]; then
  die "Error: The [$version] section in watchfaces/$face/CHANGELOG.md is empty: write it before releasing"
fi

printf '%s\n' "$body"