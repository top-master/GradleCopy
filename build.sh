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
# Default to a debug build because GradleCopy.pro's release branch pulls in
# `qt_static` Qt modules, which only exist when the XD framework was built
# with `CONFIG += static`. Override with `--release` when that's available.
mode=debug
while [ $# -gt 0 ]; do
    case "$1" in
        -v|--verbose)     verbose=1; shift;;
        --test|--tests)   run_tests=1; shift;;
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

# Build directory lives beside the source root so the source tree stays clean.
: "${BUILD_DIR:=$ROOT/build}"
mkdir -p "$BUILD_DIR"

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

# `--test` runs the standard qmake `check` target when the generated Makefile
# exposes one. The repo currently ships no tests, so on a vanilla TEMPLATE=app
# build the target is absent -- we report that cleanly instead of failing.
if [ "$run_tests" -gt 0 ]; then
    if grep -q -E '^check:' Makefile 2>/dev/null; then
        run tests make -j"$JOBS" check
    else
        echo "(no tests defined in GradleCopy.pro -- nothing to run)"
    fi
fi
