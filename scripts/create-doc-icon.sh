#!/bin/sh

# Exports the docicon.animera file to the iconset using the Animera CLI.

# Requires BUILT_PRODUCTS_DIR, EXECUTABLE_PATH and PROJECT_DIR
# Requires that Animera be built

alias animera="$BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH"
OUT="$PROJECT_DIR/Assets.xcassets/Document.iconset/"
IN="$PROJECT_DIR/Resources/Animations/docicon.animera"

animera export << EOF
{
  "output directory": "$OUT",
  "animations": [
    { "file": "$IN", "name": "icon_16x16",      "scale": 1 },
    { "file": "$IN", "name": "icon_32x32",      "scale": 2 },
    { "file": "$IN", "name": "icon_32x32@2x",   "scale": 4 },
    { "file": "$IN", "name": "icon_128x128",    "scale": 8 },
    { "file": "$IN", "name": "icon_256x256",    "scale": 16 },
    { "file": "$IN", "name": "icon_512x512",    "scale": 32 },
    { "file": "$IN", "name": "icon_512x512@2x", "scale": 64 }
  ]
}
EOF

cp "$OUT/icon_32x32.png"   "$OUT/icon_16x16@2x.png"
cp "$OUT/icon_256x256.png" "$OUT/icon_128x128@2x.png"
cp "$OUT/icon_512x512.png" "$OUT/icon_256x256@2x.png"
