#!/bin/sh

# Exports the appicon.animera file to the appiconset using the Animera CLI.

# Requires BUILT_PRODUCTS_DIR, EXECUTABLE_PATH and PROJECT_DIR
# Requires that Animera be built

alias animera="$BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH"
OUT="$PROJECT_DIR/Assets.xcassets/AppIcon.appiconset/"
IN="$PROJECT_DIR/Resources/Animations/appicon.animera"

animera export << EOF
{
  "output directory": "$OUT",
  "animations": [
    { "file": "$IN", "name": "16",   "scale": 1 },
    { "file": "$IN", "name": "32",   "scale": 2 },
    { "file": "$IN", "name": "64",   "scale": 4 },
    { "file": "$IN", "name": "128",  "scale": 8 },
    { "file": "$IN", "name": "256",  "scale": 16 },
    { "file": "$IN", "name": "512",  "scale": 32 },
    { "file": "$IN", "name": "1024", "scale": 64 }
  ]
}
EOF
