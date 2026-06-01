#!/bin/sh
# Build `assets/main.icns` from `assets/main.ico` (driven by Makefile PRE_TARGETDEPS).

set -eu

src=${1:?usage: $0 <main.ico> <main.icns>}
dest=${2:?usage: $0 <main.ico> <main.icns>}

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
iconset=$work/icon.iconset
mkdir -p "$iconset"

# sips reads .ico and writes a PNG of the largest contained entry (typically 256x256).
sips -s format png "$src" --out "$iconset/_src.png" >/dev/null

# iconset schema (`man iconutil`): one PNG per size, plus `@2x` variants.
for spec in 16:icon_16x16   32:icon_16x16@2x \
            32:icon_32x32   64:icon_32x32@2x \
            128:icon_128x128 256:icon_128x128@2x \
            256:icon_256x256 512:icon_256x256@2x \
            512:icon_512x512 1024:icon_512x512@2x; do
    size=${spec%%:*}
    name=${spec#*:}
    sips -z "$size" "$size" "$iconset/_src.png" --out "$iconset/$name.png" >/dev/null
done

rm "$iconset/_src.png"
iconutil -c icns "$iconset" -o "$dest"
