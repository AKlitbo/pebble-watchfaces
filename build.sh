#!/usr/bin/env bash
# Build watchface(s) (.pbw) from source under watchfaces/<face>/. Run from WSL.
# Regenerates the manifest from watchfaces/<face>/config/pebble.appinfo.json and compiles
# the TypeScript pkjs into targets/<face>/emit/, then runs pebble build in that sandbox.
#   ./build.sh <face>            build a face (e.g. ./build.sh lcars-stardate)
#   ./build.sh all               build every face under watchfaces/
#   ./build.sh <face> --clean    pebble clean first (needed after a messageKey change)
# Any other args forward to pebble build (e.g. ./build.sh lcars-stardate --debug).
set -euo pipefail
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ $# -lt 1 || "$1" == -* ]]; then
  echo "usage: ./build.sh <face|all> [--clean] [pebble build args...]" >&2
  exit 1
fi
face="$1"
shift

clean=0
args=()
for arg in "$@"; do
  if [[ "$arg" == "--clean" ]]; then
    clean=1
  else
    args+=("$arg")
  fi
done

build_face() {
  local face="$1"

  node --disable-warning=MODULE_TYPELESS_PACKAGE_JSON "$here/tools/manifest/build-manifests.ts" "$face"

  # compile the TypeScript pkjs runtime (watchfaces/<face>/src/pkjs + lib/ts) into
  # targets/<face>/emit/, the gitignored tree the Pebble bundler reads. the .ts is the source
  # of truth, so this runs before every build
  (cd "$here" && npm run build:pkjs -- "$face")

  echo "== building watchface $face =="
  (
    cd "$here/targets/$face"
    [[ "$clean" == 1 ]] && pebble clean
    pebble build ${args[@]+"${args[@]}"}
  )
}

if [[ "$face" == "all" ]]; then
  for dir in "$here"/watchfaces/*/; do
    build_face "$(basename "$dir")"
  done
  exit 0
fi

if [[ ! -d "$here/watchfaces/$face" ]]; then
  echo "no such face: watchfaces/$face" >&2
  exit 1
fi

build_face "$face"
