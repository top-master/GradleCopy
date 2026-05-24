#!/bin/sh

# License: Apache 2.0 without attribution need.

# MARK: Common.

# Gets the absolute path of this script's directory.
cd "${0%[/\\]*}" > /dev/null 2>&1
ROOT=$(pwd)

error() {
    echo "$ROOT/${0##*/}:${2:-1}:1: error: $1" 1>&2
    exit 1
}

# Force error handling. `pipefail` isn't in POSIX 2008 (it landed in Issue 8 /
# 2024), so try to enable it but tolerate shells that don't know it.
set -e
(set -o pipefail) 2>/dev/null && set -o pipefail || true

# MARK: Argument parsing.

verbose=0
run_tests=0
run_app=0
# Default to a debug build because GradleCopy.pro's release branch pulls in
# `qt_static` Qt modules, which only exist when the XD framework was built
# with `CONFIG += static`. Override with `--release` when that's available.
mode=debug
while [ $# -gt 0 ]; do
    case "$1" in
        -v|--verbose)     verbose=1; shift;;
        --test|--tests)   run_tests=1; shift;;
        --run)            run_app=1; shift;;
        --debug)          mode=debug; shift;;
        --release)        mode=release; shift;;
        --)               shift; break;;
        *)                break;;
    esac
done

# MARK: Defaults.

# Parallel-job count for make. Override via the JOBS env var. Picks the host's
# logical CPU count using whichever of getconf/sysctl/nproc is present; falls
# back to 4 on shells that lack all three.
if [ -z "${JOBS:-}" ]; then
    JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null \
           || sysctl -n hw.logicalcpu 2>/dev/null \
           || nproc 2>/dev/null \
           || echo 4)
fi

# Pick the mkspec by host. macx-clang on Darwin, linux-g++ otherwise.
case "$(uname -s)" in
    Darwin) : "${QMAKESPEC:=macx-clang}";;
    *)      : "${QMAKESPEC:=linux-g++}";;
esac

# On Darwin, qmake's `mac/default_pre.prf` shells out to `xcode-select` and
# `xcrun` to locate Xcode. When the active selection is the bare Command Line
# Tools tree (no `xcodebuild`, no SDK metadata) but a real Xcode.app exists,
# export DEVELOPER_DIR pointing at it so those tools resolve correctly --
# saves the user from running `sudo xcode-select -switch ...` system-wide.
if [ "$(uname -s)" = "Darwin" ]; then
    _xs_path=$(xcode-select --print-path 2>/dev/null || true)
    if [ ! -x "$_xs_path/usr/bin/xcodebuild" ] && [ -z "${DEVELOPER_DIR:-}" ]; then
        for _xc in /Applications/Xcode*.app; do
            [ -d "$_xc/Contents/Developer" ] || continue
            DEVELOPER_DIR=$_xc/Contents/Developer
            export DEVELOPER_DIR
            break
        done
    fi
fi

# MARK: Resolve XD framework.

# Honor $XD_ROOT when set; otherwise fall back to a sibling `../XD` checkout
# next to this repo. Both forms get resolved to an absolute path so later
# `cd` calls don't depend on where the script was invoked from.
if [ -n "${XD_ROOT:-}" ]; then
    XD_DIR=$XD_ROOT
else
    XD_DIR=$ROOT/../XD
fi
if [ ! -d "$XD_DIR" ]; then
    error "XD framework not found at \"$XD_DIR\" (set XD_ROOT to override)"
fi
XD_DIR=$(cd "$XD_DIR" && pwd)

QMAKE=$XD_DIR/bin/qmake
if [ ! -x "$QMAKE" ]; then
    error "XD qmake not found at \"$QMAKE\" -- build the XD framework first"
fi

# Build directory lives as a sibling to the source root, under a shared
# `build/` folder named after this repo plus the build mode. Keeping the
# mode in the dir name lets debug and release artifacts coexist without
# stomping each other (the underlying Makefile carries different
# QMAKE_CXXFLAGS, library paths, and link names per mode).
: "${BUILD_DIR:=$(dirname "$ROOT")/build/$(basename "$ROOT")-$mode}"
mkdir -p "$BUILD_DIR"

# Stage a per-mode plugin search dir holding only the variant matching
# this build, then point QT_*_PLUGIN_PATH at it for any later launch.
#
# XD ships both debug and release copies of every plugin (libqcocoa.dylib
# *and* libqcocoa_debug.dylib, etc.). `QFactoryLoader` walks the whole
# search directory and `dlopen`s every candidate just to read its
# metadata; on macOS that `dlopen` runs the plugin's Objective-C `+load`
# methods and pulls in its linked Qt5*.dylib variant. So even though
# the debug-vs-release check in `qlibrary.cpp` ultimately rejects the
# wrong-variant plugin, both `libQt5Core.dylib` and
# `libQt5Core_debug.dylib` end up in the process, the duplicate
# Objective-C class registrations clash ("Class RunLoopModeTracker is
# implemented in both ..."), and the app segfaults during
# `QObject::moveToThread`.
QT_PLUGINS_STAGE=$BUILD_DIR/qt-plugins-$mode
rm -rf "$QT_PLUGINS_STAGE"
for _src_dir in "$XD_DIR/plugins"/*/; do
    _type=$(basename "$_src_dir")
    _dest=$QT_PLUGINS_STAGE/$_type
    mkdir -p "$_dest"
    for _src in "$_src_dir"*.dylib; do
        [ -f "$_src" ] || continue
        _base=$(basename "$_src")
        case "$mode" in
            debug)
                case "$_base" in *_debug.dylib) ln -sf "$_src" "$_dest/$_base";; esac;;
            release)
                case "$_base" in *_debug.dylib) ;; *) ln -sf "$_src" "$_dest/$_base";; esac;;
        esac
    done
done

# MARK: Run helper.

_step=0

# Per-step logs go to disk (no risk of overflowing a shell variable for a long
# build). Each invocation is a fresh re-build, so wipe the prior logs up
# front -- only deletes existing `*.log` files; the folder itself is reused.
LOGS=$BUILD_DIR/logs
mkdir -p "$LOGS"
rm -f "$LOGS"/*.log

# Run a named step. Stream goes to `$LOGS/<NN>-<name>.log`; verbose mode also
# tees it to the terminal. Non-verbose mode is silent on success and dumps the
# step's log to stderr only on failure (preserving the original exit status).
run() {
    _name=$1; shift
    _step=$((_step + 1))
    _log=$LOGS/$(printf '%02d-%s.log' "$_step" "$_name")
    if [ "$verbose" -gt 0 ]; then
        "$@" 2>&1 | tee "$_log"
    else
        if "$@" >"$_log" 2>&1; then
            :
        else
            _status=$?
            cat "$_log" 1>&2
            exit "$_status"
        fi
    fi
}

# MARK: Build.

cd "$BUILD_DIR"

run qmake "$QMAKE" -spec "$XD_DIR/mkspecs/$QMAKESPEC" "$ROOT/GradleCopy.pro" \
    CONFIG+="$mode" "$@"
run make  make -j"$JOBS"

# MARK: Tests.

# `--test` builds and runs the dedicated test runner under `test/test.pro`,
# which itself pulls in `GradleCopy.pro`'s sources (minus `main.cpp`) plus
# `CONFIG += testcase`, so qmake emits a real `make check` rule that execs
# the test binary. We use a separate build subtree so the regular app build
# above and the test build don't trample each other's Makefile.
#
# Falls back to the original "no testcase" message when `test/test.pro` is
# missing, so the script remains useful on a stripped checkout.
if [ "$run_tests" -gt 0 ]; then
    if [ -f "$ROOT/test/test.pro" ]; then
        # The test runner installs itself into the root app's existing
        # `.app/Contents/MacOS` rather than producing its own `.app`
        # bundle. That contract only holds when the root app actually
        # built first -- assert the host bundle is on disk before letting
        # qmake/make populate paths under it.
        case "$(uname -s)" in
            Darwin)
                _host_bin=$BUILD_DIR/bin/GradleCopy.app/Contents/MacOS/GradleCopy
                if [ ! -x "$_host_bin" ]; then
                    error "root app bundle missing at \"$_host_bin\" --" \
                        "the test runner installs into the existing" \
                        "GradleCopy.app, so the regular build must" \
                        "complete first"
                fi;;
        esac

        TEST_BUILD=$BUILD_DIR/test
        mkdir -p "$TEST_BUILD"
        cd "$TEST_BUILD"
        run test-qmake "$QMAKE" -spec "$XD_DIR/mkspecs/$QMAKESPEC" \
            "$ROOT/test/test.pro" CONFIG+="$mode"
        run test-make  make -j"$JOBS"
        cd "$BUILD_DIR"

        # `make check` would call `target_wrapper.sh ./...test-runner.app`
        # via qmake's testcase mkspec, but that wrapper isn't shipped with
        # XD. Exec the test binary directly with the same DYLD/plugin env
        # that `--run` uses so the linked Qt5 dylibs and the cocoa platform
        # plugin resolve cleanly.
        #
        # On macOS the runner is installed as a plain binary inside the
        # main `GradleCopy.app` bundle (rather than its own `.app`) so the
        # build doesn't duplicate the bundle's resources/plugins for tests.
        if [ -x "$BUILD_DIR/bin/GradleCopy.app/Contents/MacOS/GradleCopy-test-runner" ]; then
            TEST_BIN=$BUILD_DIR/bin/GradleCopy.app/Contents/MacOS/GradleCopy-test-runner
        elif [ -x "$BUILD_DIR/bin/GradleCopy-test-runner" ]; then
            TEST_BIN=$BUILD_DIR/bin/GradleCopy-test-runner
        elif [ -x "$BUILD_DIR/bin/GradleCopy-test-runner.exe" ]; then
            TEST_BIN=$BUILD_DIR/bin/GradleCopy-test-runner.exe
        else
            error "test runner not found under \"$BUILD_DIR/bin\""
        fi
        run tests env \
            DYLD_LIBRARY_PATH="$XD_DIR/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}" \
            LD_LIBRARY_PATH="$XD_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
            QT_PLUGIN_PATH="$QT_PLUGINS_STAGE${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}" \
            QT_QPA_PLATFORM_PLUGIN_PATH="$QT_PLUGINS_STAGE/platforms" \
            "$TEST_BIN"
    else
        _step=$((_step + 1))
        printf '(no test/test.pro found -- nothing to run)\n' \
            | tee "$LOGS/$(printf '%02d-%s.log' "$_step" tests-skipped)"
    fi
fi

# MARK: Launch.

# `--run` boots the freshly-built artifact and keeps its stdout/stderr
# attached to this terminal so `qDebug()` output is visible.
#
# On macOS the build produces a `GradleCopy.app` bundle; we exec the inner
# binary rather than handing the bundle to `open`, because XD's dylibs ship
# with bare-name install paths (no @rpath, no absolute prefix) and `open`
# routes through launchd which strips `DYLD_*` env vars for security --
# the app would launch, fail to dlopen its Qt libs, and die silently. The
# direct-exec path lets us inject DYLD_LIBRARY_PATH and Qt's plugin-search
# vars so the cocoa platform plugin and the linked dylibs all resolve.
if [ "$run_app" -gt 0 ]; then
    if [ -x "$BUILD_DIR/bin/GradleCopy.app/Contents/MacOS/GradleCopy" ]; then
        BIN=$BUILD_DIR/bin/GradleCopy.app/Contents/MacOS/GradleCopy
    elif [ -x "$BUILD_DIR/bin/GradleCopy" ]; then
        BIN=$BUILD_DIR/bin/GradleCopy
    elif [ -x "$BUILD_DIR/bin/GradleCopy.exe" ]; then
        BIN=$BUILD_DIR/bin/GradleCopy.exe
    else
        error "no GradleCopy executable found under \"$BUILD_DIR/bin\""
    fi
    echo "Launching $BIN"
    DYLD_LIBRARY_PATH=$XD_DIR/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH} \
    LD_LIBRARY_PATH=$XD_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} \
    QT_PLUGIN_PATH=$QT_PLUGINS_STAGE${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH} \
    QT_QPA_PLATFORM_PLUGIN_PATH=$QT_PLUGINS_STAGE/platforms \
    "$BIN"
fi
