#!/bin/sh

# License: Apache 2.0 without attribution need.

# MARK: Common.

# Gets the absolute path of this script's directory.
cd "${0%[/\\]*}" > /dev/null 2>&1
ROOT=$(pwd)

# MARK: Resolve XD framework.
#
# Locate XD (the directory carrying `XD-mini.pro`). Honor $XD_ROOT first,
# then sibling checkouts -- upper- and lower-case, the 5.6 "XD5" and plain
# "XD" spellings, each with or without a `qtbase` subdir. If none is found,
# offer to shallow-clone the 5.6 branch; ask first, unless the run is
# non-interactive. Mirrors ~/projects/xkalq-server/find-xd.sh.
#
# Overridable: XD_REPO_URL (default the public XD repo), XD_BRANCH (5.6).
XD_REPO_URL=${XD_REPO_URL:-https://github.com/top-master/XD.git}
XD_BRANCH=${XD_BRANCH:-5.6}

# Is $1 a usable checkout? A non-git tree is accepted (with a note); a git
# checkout must have a valid HEAD and no merge/rebase in progress.
xd_git_check() {
    if ! git -C "$1" rev-parse --git-dir > /dev/null 2>&1; then
        echo "build.sh: note: \"$1\" is not a git checkout; skipping git health check" 1>&2
        return 0
    fi
    if ! git -C "$1" rev-parse --verify -q HEAD > /dev/null 2>&1; then
        echo "$ROOT/build.sh:1:1: error: XD at \"$1\" has no valid git HEAD" 1>&2
        return 1
    fi
    if git -C "$1" rev-parse -q --verify MERGE_HEAD > /dev/null 2>&1; then
        echo "$ROOT/build.sh:1:1: error: XD at \"$1\" has a merge in progress" 1>&2
        return 1
    fi
    _gd=$(git -C "$1" rev-parse --absolute-git-dir 2>/dev/null)
    if [ -d "$_gd/rebase-merge" ] || [ -d "$_gd/rebase-apply" ]; then
        echo "$ROOT/build.sh:1:1: error: XD at \"$1\" has a rebase in progress" 1>&2
        return 1
    fi
    return 0
}

XD_DIR=""
for _c in \
    "${XD_ROOT:-}" \
    "$ROOT/../XD5/qtbase" \
    "$ROOT/../XD/qtbase" \
    "$ROOT/../XD5" \
    "$ROOT/../XD" \
    "$ROOT/../xd5/qtbase" \
    "$ROOT/../xd/qtbase" \
    "$ROOT/../xd5" \
    "$ROOT/../xd" \
; do
    if [ -n "$_c" ] && [ -f "$_c/XD-mini.pro" ]; then
        XD_DIR=$(cd "$_c" && pwd)
        break
    fi
done

if [ -z "$XD_DIR" ]; then
    # Decide interactivity the same way build-handler.sh will: BH_HEADLESS
    # (default 0) flipped by --headless/--test-review or --headed/--interactive.
    # PEEK at the flags -- do not shift them, so `bh_template_app "$@"` below
    # still receives the full, unmodified argument list.
    _headless=${BH_HEADLESS:-0}
    for _a in "$@"; do
        case "$_a" in
            --headless|--test-review) _headless=1 ;;
            --headed|--interactive)   _headless=0 ;;
        esac
    done

    _target="$ROOT/../XD5"
    if ! command -v git > /dev/null 2>&1; then
        echo "$ROOT/build.sh:1:1: error: XD framework not found and git is unavailable to clone it (set XD_ROOT to an existing XD checkout)" 1>&2
        exit 1
    fi
    if [ -e "$_target" ]; then
        echo "$ROOT/build.sh:1:1: error: XD framework not found, but \"$_target\" already exists (remove it, or set XD_ROOT to a valid XD)" 1>&2
        exit 1
    fi
    if [ "$_headless" -eq 0 ]; then
        printf 'XD framework not found. Clone %s (branch %s) into "%s"? [Y/n] ' \
            "$XD_REPO_URL" "$XD_BRANCH" "$_target"
        read -r _ans
        case "$_ans" in
            n | N | no | NO | No)
                echo "$ROOT/build.sh:1:1: error: XD framework not found; aborted (set XD_ROOT to an existing XD checkout)" 1>&2
                exit 1
                ;;
        esac
    else
        echo "build.sh: XD framework not found; shallow-cloning $XD_BRANCH from $XD_REPO_URL into $_target ..." 1>&2
    fi
    if git clone --depth 1 --branch "$XD_BRANCH" "$XD_REPO_URL" "$_target" 1>&2 \
        && [ -f "$_target/XD-mini.pro" ]; then
        XD_DIR=$(cd "$_target" && pwd)
    else
        echo "$ROOT/build.sh:1:1: error: clone of $XD_BRANCH from $XD_REPO_URL into \"$_target\" failed" 1>&2
        exit 1
    fi
fi

# Validate the checkout (found or self-cloned) before handing off.
xd_git_check "$XD_DIR" || exit 1

# MARK: Hand off to the shared template driver.
#
# Everything from here on -- arg parsing, build dir, plugin staging,
# qmake, make, tests, launch -- lives under
# `$XD_DIR/tools/build-handler.sh`'s `bh_template_app`. This script
# only owns the XD-detection bit above and the .pro path below.

BH_ROOT=$ROOT
BH_SCRIPT_NAME=build.sh
. "$XD_DIR/tools/build-handler.sh"

bh_template_app "$ROOT/GradleCopy.pro" "$@"
