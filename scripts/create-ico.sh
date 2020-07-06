#!/bin/sh

# Creates the Windows ico files from the appiconset and iconset

# Requires PROJECT_DIR
# Requires icotool from icoutils
# Requires gm (graphicsmagick)

APPICONS="$PROJECT_DIR/Assets.xcassets/AppIcon.appiconset"
gm convert "$APPICONS/32.png" -resize 24 24.png
gm convert "$APPICONS/16.png" -filter Point -resize 48 48.png
icotool -c "$APPICONS/16.png" 24.png "$APPICONS/32.png" 48.png "$APPICONS/64.png" "$APPICONS/128.png" "$APPICONS/256.png" -o "$PROJECT_DIR/Resources/Logo/appicon.ico"
rm 24.png 48.png

DOCICONS="$PROJECT_DIR/Assets.xcassets/Document.iconset"
gm convert "$DOCICONS/icon_32x32.png" -resize 24 24.png
gm convert "$DOCICONS/icon_16x16.png" -filter Point -resize 48 48.png
icotool -c "$DOCICONS/icon_16x16.png" 24.png "$DOCICONS/icon_32x32.png" 48.png "$DOCICONS/icon_32x32@2x.png" "$DOCICONS/icon_128x128.png" "$DOCICONS/icon_256x256.png" -o "$PROJECT_DIR/Resources/Logo/docicon.ico"
rm 24.png 48.png
