import os
import shutil
import json


def _repo_root(ctx):
    """
    The repo root: the nearest ancestor of the face dir holding lib/ (the reusable
    base). Found by walking up rather than assuming a fixed depth, so the layout can
    move without breaking every wscript. The marker is name-agnostic to the face
    folder, so renaming it never breaks this.
    """
    node = ctx.path
    while node is not None:
        if node.find_dir('lib'):
            return node
        node = node.parent

    ctx.fatal('waf_helpers: could not locate the repo root (no ancestor with lib/)')


def build_conditions(ctx):
    """
    Regenerate the weather-icon lookup (icons_table.g.h) from the shared condition
    vocabulary in lib/ts/weather/conditions.ts. Non-fatal: the generated header is
    committed, so a node-less environment still builds with the last-generated table.
    """
    from waflib import Logs

    # the tools are ESM .ts in a package with no "type", which is what keeps the tsc-emitted
    # runtime .js CommonJS for the bundler. node detects the module type from the syntax and
    # warns about it, so silence just that warning rather than declare a type
    script = _repo_root(ctx).find_node('lib/tools/build-conditions.ts')
    cmd = ['node', '--disable-warning=MODULE_TYPELESS_PACKAGE_JSON', script.abspath()] if script else None
    if script and ctx.exec_command(cmd) != 0:
        Logs.warn('build-conditions: node unavailable; using committed icons_table.g.h')


def stage_shared_sources(ctx):
    """
    Mirror this face's src/ and resources/ plus the shared lib/ into this build folder
    (targets/<face>/) so the SDK sees a normal, self-contained project. The sandbox is
    named after the face, so ctx.path.name is the face; its src/ and resources/ live under
    watchfaces/<face>/, while lib/ is shared at the repo root and comes along for the C.

    emit/ is not staged: build:pkjs writes it straight into this sandbox (targets/<face>/emit)
    keeping the watchfaces/<face>/src/pkjs + lib/ts layout, so the entry's relative requires
    (../../../../lib/ts) already resolve here.

    Only files whose size or mtime differ are copied, and mtimes are preserved, so an
    untouched rebuild does not force a full recompile. Staged files whose source is
    gone are dropped. The roots come from ctx.path.parent.parent (the true repo root, since
    this folder sits under targets/), not _repo_root, because once lib/ is staged here
    _repo_root would resolve to this folder instead.
    """
    repo_root = ctx.path.parent.parent.abspath()
    face = ctx.path.name
    face_root = _face_root(repo_root, face)
    if not face_root:
        ctx.fatal('No source for face "{}" under watchfaces/.'.format(face))

    sources = {
        'src': os.path.join(face_root, 'src'),
        'resources': os.path.join(face_root, 'resources'),
        'lib': os.path.join(repo_root, 'lib'),
    }

    # a face nested inside a family folder also gets that family's core: code shared by a handful
    # of related faces but not by all of them, so it cannot live in lib/. it is staged under the
    # family's own name, and that name is a path segment the build synthesises rather than one
    # anybody types, which is what keeps a family header from colliding with a face-local folder
    family = _face_family(repo_root, face_root)
    if family:
        sources[os.path.join('family', family)] = os.path.join(
            repo_root, 'watchfaces', family, 'core', 'c')

    for name, src_dir in sources.items():
        if os.path.isdir(src_dir):
            _mirror_tree(src_dir, os.path.join(ctx.path.abspath(), name))

    _drop_stale_families(ctx.path.abspath(), family)


def _face_root(repo_root, face):
    """
    A face's source directory, by name.

    Faces sit either straight under watchfaces/ or one deeper inside a family folder that also
    holds the code those faces share. A face is a directory carrying config/pebble.appinfo.json,
    which is what keeps a family's core/ from being taken for one.
    """
    watchfaces = os.path.join(repo_root, 'watchfaces')
    for candidate in (os.path.join(watchfaces, face),
                      *(os.path.join(watchfaces, group, face)
                        for group in sorted(os.listdir(watchfaces))
                        if os.path.isdir(os.path.join(watchfaces, group)))):
        if os.path.isfile(os.path.join(candidate, 'config', 'pebble.appinfo.json')):
            return candidate

    return None


def _face_family(repo_root, face_root):
    """
    The family a face belongs to, which is simply the folder it sits in.

    Nothing declares it: a face nested beside a core/ is in that family, and one straight under
    watchfaces/ is in none. Positional rather than configured, so the two can never disagree.
    """
    parent = os.path.dirname(face_root)
    if os.path.samefile(parent, os.path.join(repo_root, 'watchfaces')):
        return None

    return os.path.basename(parent)


def _drop_stale_families(sandbox, family):
    """
    Drop any staged family that is not this face's any more.

    _mirror_tree only prunes inside the one root it is handed, so a face that changed families
    or left one would keep the old tree staged and the build would go on compiling it.
    """
    staged = os.path.join(sandbox, 'family')
    if not os.path.isdir(staged):
        return

    for name in os.listdir(staged):
        if name != family:
            shutil.rmtree(os.path.join(staged, name), ignore_errors=True)


def _mirror_tree(src, dst):
    """Copy src into dst, refreshing changed files and dropping ones src no longer has."""
    if not os.path.isdir(dst):
        os.makedirs(dst)

    # add or refresh anything that differs
    for root, _dirs, files in os.walk(src):
        rel = os.path.relpath(root, src)
        dst_root = dst if rel == '.' else os.path.join(dst, rel)
        if not os.path.isdir(dst_root):
            os.makedirs(dst_root)
        for name in files:
            src_file = os.path.join(root, name)
            dst_file = os.path.join(dst_root, name)
            if _needs_copy(src_file, dst_file):
                shutil.copy2(src_file, dst_file)

    # drop staged files (and now-empty dirs) whose source has gone away
    for root, dirs, files in os.walk(dst, topdown=False):
        rel = os.path.relpath(root, dst)
        src_root = src if rel == '.' else os.path.join(src, rel)
        for name in files:
            if not os.path.exists(os.path.join(src_root, name)):
                os.remove(os.path.join(root, name))
        for name in dirs:
            staged = os.path.join(root, name)
            if not os.path.isdir(os.path.join(src_root, name)) and not os.listdir(staged):
                os.rmdir(staged)


def _needs_copy(src_file, dst_file):
    """True when dst is missing or differs from src in size or whole-second mtime."""
    if not os.path.exists(dst_file):
        return True
    src_stat = os.stat(src_file)
    dst_stat = os.stat(dst_file)
    return (src_stat.st_size != dst_stat.st_size
            or int(src_stat.st_mtime) != int(dst_stat.st_mtime))


def build_face(ctx, extra_cflags=None):
    """
    The build: resolve include paths, collect the face's C/JS plus the shared lib/,
    compile the app per target platform, bundle with PebbleKit JS, and archive the
    .pbw afterward. extra_cflags are appended to every platform's CFLAGS (the watchapp
    build passes -DBUILD_WATCHAPP).
    """
    repo = _repo_root(ctx)

    # lib/ = the reusable base copied into every face. lib/c/core/ is pure (SDK-free and
    # host-testable). lib/c/pebble/ needs the SDK. the PebbleKit JS is compiled out of
    # lib/ts/ into emit/ before the build, so only the C comes from here
    lib_dir = repo.find_dir('lib')
    if not lib_dir:
        ctx.fatal('Could not find the "lib" directory at the repo root.')

    lib_c = lib_dir.find_dir('c')
    lib_c_core = lib_c.find_dir('core')
    lib_c_pebble = lib_c.find_dir('pebble')
    # this face's own sources (grid engine, widgets, main, theme)
    local_c = ctx.path.find_dir('src/c')

    # only the two lib roots + the face-local dir are on the include path. every shared
    # header is included with its folder relative to a root (e.g. "ui/engine/engine.h" or
    # "clock/beats.h") so moving a folder never touches this list
    include_paths = [
        lib_c_core.abspath(),
        lib_c_pebble.abspath(),
        local_c.abspath()
    ]

    # the family root, for a face that belongs to one (see stage_shared_sources). it goes on
    # last so a face-local header always wins, and family headers carry the family's name as
    # their first path segment (e.g. "line/sky/sky.h") so they cannot shadow the face's own
    # scene/, theme/ or widgets/
    family_dir = ctx.path.find_dir('family')
    if family_dir:
        include_paths.append(family_dir.abspath())

    # one glob recurses the whole shared tree: lib/c/core/ (pure) + lib/c/pebble/ (SDK).
    # drop the colocated host tests (*.spec.c) and the vendored test harness (spec/) so
    # they never spend a byte on the watch
    c_sources = ctx.path.ant_glob('src/c/**/*.c') + lib_c.ant_glob(
        '**/*.c', excl=['**/*.spec.c', 'spec/**'])

    if family_dir:
        c_sources += family_dir.ant_glob('**/*.c', excl=['**/*.spec.c'])

    # emit/ is build:pkjs output and holds nothing but the JS the watch ships: the specs
    # and the clay/builder pieces are dropped at the tsc level (config/tsconfig.pkjs.json),
    # and the generated *.g.js components are copied in beside it. so the whole tree goes
    js_sources = ctx.path.ant_glob('emit/**/*.js')

    binaries = []
    cached_env = ctx.env

    cflags = []
    pkg_node = ctx.path.find_node('package.json')
    if pkg_node:
        try:
            with open(pkg_node.abspath(), 'r') as pkg_f:
                pkg_data = json.load(pkg_f)
                mkeys = pkg_data.get('pebble', {}).get('messageKeys', [])
                if isinstance(mkeys, list):
                    for mk in mkeys:
                        if isinstance(mk, str):
                            cflags.append('-DHAS_MESSAGE_KEY_' + mk + '=1')
                elif isinstance(mkeys, dict):
                    for mk in mkeys.keys():
                        cflags.append('-DHAS_MESSAGE_KEY_' + mk + '=1')

                # the shared weather-icon lookup (lib/c/.../ui/weather/icons.c) is gated on
                # HAS_WEATHER_ICONS. it references the ICON_WEATHER_NOW_* set via generated
                # tables, so only a face that ships those icons should compile it. the whole
                # set travels together, so the always-present fallback is a reliable proxy.
                media = pkg_data.get('pebble', {}).get('resources', {}).get('media', [])
                if isinstance(media, list) and any(
                        isinstance(m, dict) and m.get('name') == 'ICON_WEATHER_NOW_NA' for m in media):
                    cflags.append('-DHAS_WEATHER_ICONS=1')
        except Exception:
            pass

    if extra_cflags:
        cflags.extend(extra_cflags)

    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)

        ctx.env.append_value('INCLUDES', include_paths)
        if cflags:
            ctx.env.append_value('CFLAGS', cflags)

        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)

        ctx.pbl_build(
            source=c_sources,
            target=app_elf,
            bin_type='app'
        )

        binaries.append({'platform': platform, 'app_elf': app_elf})

    ctx.env = cached_env

    # emit/ keeps the source tree's shape, so the entry sits wherever the face's pkjs does. that
    # is one folder deep for a face of its own and two for one inside a family, so it is found
    # rather than assumed
    entries = ctx.path.ant_glob('emit/watchfaces/**/src/pkjs/index.js')
    if not entries:
        ctx.fatal('No pkjs entry in emit/: did build:pkjs run for this face?')
    js_entry = entries[0].path_from(ctx.path)

    ctx.set_group('bundle')
    ctx.pbl_bundle(
        binaries=binaries,
        js=js_sources,
        js_entry_file=js_entry
    )
