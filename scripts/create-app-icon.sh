#!/bin/sh

# Exports the appicon.animera file to the appiconset using the Animera CLI.

# Requires BUILT_PRODUCTS_DIR, EXECUTABLE_PATH and PROJECT_DIR
# Requires that Animera be built

alias animera="$BUILT_PRODUCTS_DIR/$EXECUTABLE_PATH"
OUT="$PROJECT_DIR/Assets.xcassets/AppIcon.appiconset/"
IN="$PROJECT_DIR/Resources/Animations/appicon.animera"

function ex {
  animera export --name="$1" --directory="$OUT" --no-composite --scale="$2" "$IN"
}

ex 16 1
ex 32 2
ex 64 4
ex 128 8
ex 256 16
ex 512 32
ex 1024 64
