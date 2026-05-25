#!/bin/sh

# License: Apache 2.0 without attribution need.

# MARK: Common.

# Gets the absolute path of this script's directory.
cd "${0%[/\\]*}" > /dev/null 2>&1
ROOT=$(pwd)

# MARK: Resolve XD framework.

# Honor $XD_ROOT when set; otherwise fall back to a sibling `../XD`
# checkout. Resolve to absolute, so later `cd` calls work and the
# shared build-handler script can be sourced from XD/tools/.
if [ -n "${XD_ROOT:-}" ]; then
    XD_DIR=$XD_ROOT
else
    XD_DIR=$ROOT/../XD
fi
if [ ! -d "$XD_DIR" ]; then
    # XD isn't located yet, so we can't use bh_error here; emit the same
    # `<script>:line:col: error:` shape inline.
    echo "$ROOT/build.sh:1:1: error: XD framework not found at \"$XD_DIR\" (set XD_ROOT to override)" 1>&2
    exit 1
fi
XD_DIR=$(cd "$XD_DIR" && pwd)

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
