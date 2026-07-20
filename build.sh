#!/usr/bin/env bash
# Build watchface(s) (.pbw) from source under watchfaces/. A face may sit at the top level
# or one deeper inside a family folder. Run from WSL.
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

  # a face usually builds one target (the face itself), but can declare several (a watchface
  # and a watchapp from one source). the manifest step wrote a sandbox per target; ask it which
  local targets
  targets=$(node --disable-warning=MODULE_TYPELESS_PACKAGE_JSON "$here/tools/manifest/build-manifests.ts" --targets "$face")

  for target in $targets; do
    # compile the TypeScript pkjs runtime (watchfaces/<face>/src/pkjs + lib/ts) into
    # targets/<target>/emit/, the gitignored tree the Pebble bundler reads. the .ts is the
    # source of truth, so this runs before every build
    (cd "$here" && npm run build:pkjs -- "$target" "$face")

    echo "== building $target (face $face) =="
    (
      cd "$here/targets/$target"
      [[ "$clean" == 1 ]] && pebble clean
      pebble build ${args[@]+"${args[@]}"}
    )
  done
}

# a face is any directory under watchfaces/ carrying an appinfo, at the top level or one deeper
# inside a family folder that also holds the code its faces share. that rule is what keeps a
# family's core/ from being built as a face, with nothing to register anywhere
face_dirs() {
  find "$here/watchfaces" -mindepth 3 -maxdepth 4 -name pebble.appinfo.json -path '*/config/*'     | sed 's#/config/pebble.appinfo.json$##' | sort
}

if [[ "$face" == "all" ]]; then
  while IFS= read -r dir; do
    build_face "$(basename "$dir")"
  done < <(face_dirs)
  exit 0
fi

if ! face_dirs | grep -qx ".*/$face"; then
  echo "no such face: nothing under watchfaces/ carries $face/config/pebble.appinfo.json" >&2
  exit 1
fi

build_face "$face"
