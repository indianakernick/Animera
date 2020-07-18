#!/bin/sh

# Exports the docicon.animera file to the iconset using the Animera CLI.

# Requires BUILT_PRODUCTS_DIR, EXECUTABLE_PATH and PROJECT_DIR
# Requires that Animera be built

alias animera="$BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH"
OUT="$PROJECT_DIR/Assets.xcassets/Document.iconset/"
IN="$PROJECT_DIR/Resources/Animations/docicon.animera"

function ex {
  animera export --name="$1" --directory="$OUT" --scale="$2" "$IN"
}

ex icon_16x16 1
ex icon_32x32 2
ex icon_32x32@2x 4
ex icon_128x128 8
ex icon_256x256 16
ex icon_512x512 32
ex icon_512x512@2x 64

cp "$OUT/icon_32x32.png"   "$OUT/icon_16x16@2x.png"
cp "$OUT/icon_256x256.png" "$OUT/icon_128x128@2x.png"
cp "$OUT/icon_512x512.png" "$OUT/icon_256x256@2x.png"
