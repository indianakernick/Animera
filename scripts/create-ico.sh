#!/bin/sh

# Creates the Windows ico files from the appiconset and iconset

# Requires PROJECT_DIR
# Requires icotool from icoutils
# Requires gm (graphicsmagick)

APPICONS="$PROJECT_DIR/Assets.xcassets/AppIcon.appiconset"
icotool -c "$APPICONS/16.png" "$APPICONS/32.png" "$APPICONS/64.png" "$APPICONS/128.png" "$APPICONS/256.png" -o "$PROJECT_DIR/Resources/Logo/appicon.ico"

DOCICONS="$PROJECT_DIR/Assets.xcassets/Document.iconset"
icotool -c "$DOCICONS/icon_16x16.png" "$DOCICONS/icon_32x32.png" "$DOCICONS/icon_32x32@2x.png" "$DOCICONS/icon_128x128.png" "$DOCICONS/icon_256x256.png" -o "$PROJECT_DIR/Resources/Logo/docicon.ico"
