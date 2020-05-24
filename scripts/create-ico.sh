#!/bin/sh

# Creates the Windows ico file from the appiconset.

# Requires PROJECT_DIR
# Requires icotool from icoutils

ICONS="$PROJECT_DIR/Assets.xcassets/AppIcon.appiconset"
icotool -c "$ICONS/16.png" "$ICONS/32.png" "$ICONS/64.png" "$ICONS/128.png" "$ICONS/256.png" -o "$PROJECT_DIR/Resources/Logo/icon.ico"
