#!/usr/bin/env bash
# Aggregate Unity's per-spec results into one pass/fail summary.
#
# Every *.spec.c under lib/c is its own standalone Unity program, so `make -C lib/c/spec` prints
# a "N Tests M Failures K Ignored" counter line per spec rather than one total for the run. This
# adds them up. Run it after the suite, from WSL. On CI (GITHUB_STEP_SUMMARY set) it also appends
# the same markdown to the run's summary page, alongside the image headroom report.
#
# Reporting only: the suite itself is the gate (make exits non-zero when a spec fails), so this
# never fails a run over test results. It does fail when handed a log with no results at all.
#
# Usage: tools/ci/report-c-tests.sh [log]   (defaults to c-tests.log)

set -euo pipefail

# helper to print errors to stderr and exit
die() {
  echo "$1" >&2
  exit 1
}

log="${1:-c-tests.log}"
counter='[0-9]+ Tests [0-9]+ Failures [0-9]+ Ignored'

[[ -f "$log" ]] || die "Error: No test log at $log. Run 'make -C lib/c/spec | tee $log' first."

# a suite that failed to compile logs no counter lines at all, which would total to zero and
# read as an all-green report. Treat an empty result set as a broken run, not a passing one.
if ! grep -qE "$counter" "$log"; then
  die "Error: No Unity results in $log. The suite did not run."
fi

# one counter line per spec binary, so the line count is the file count. $1/$3/$5 are the
# Tests/Failures/Ignored numbers off "19 Tests 0 Failures 0 Ignored"
report=$(awk -v regex="$counter" '
  $0 ~ regex {
    files++
    total += $1
    fails += $3
    ignored += $5
    if ($3 == 0) { filespass++ }
  }
  END {
    passes = total - fails - ignored
    filemark = (filespass == files) ? "✅" : "❌"
    resultmark = (fails == 0) ? "✅" : "❌"
    
    print "## Unity C Test Report\n"
    print "### Summary\n"
    printf "- **Test Files:** %s **%d passes** · %d total\n", filemark, filespass, files
    printf "- **Test Results:** %s **%d passes** · %d total\n", resultmark, passes, total
    printf "- **Other:** %d skips · %d total\n", ignored, ignored
  }
' "$log")

printf '%s\n' "$report"

# on CI the same report lands on the run's summary page next to the headroom table
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
  printf '%s\n' "$report" >> "$GITHUB_STEP_SUMMARY"
fi