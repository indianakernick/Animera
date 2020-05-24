#!/bin/sh

# Creates the document icon by compositing the app icon with the generic
# document icon template.

# Requires PROJECT_DIR
# Requires gm (graphicsmagick)

DOC_ICON="$PROJECT_DIR/Assets.xcassets/Document.iconset"
APP_ICON="$PROJECT_DIR/Assets.xcassets/AppIcon.appiconset"

iconutil --convert iconset --output template.iconset /System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/GenericDocumentIcon.icns

gm convert -resize 8x8 "$APP_ICON/16.png" 8.png

gm composite -geometry +4+6     8.png               template.iconset/icon_16x16.png      "$DOC_ICON/icon_16x16.png"
gm composite -geometry +8+12    "$APP_ICON/16.png"  template.iconset/icon_32x32.png      "$DOC_ICON/icon_32x32.png"
gm composite -geometry +16+24   "$APP_ICON/32.png"  template.iconset/icon_32x32@2x.png   "$DOC_ICON/icon_32x32@2x.png"
gm composite -geometry +32+48   "$APP_ICON/64.png"  template.iconset/icon_128x128.png    "$DOC_ICON/icon_128x128.png"
gm composite -geometry +64+96   "$APP_ICON/128.png" template.iconset/icon_256x256.png    "$DOC_ICON/icon_256x256.png"
gm composite -geometry +128+192 "$APP_ICON/256.png" template.iconset/icon_512x512.png    "$DOC_ICON/icon_512x512.png"
gm composite -geometry +256+384 "$APP_ICON/512.png" template.iconset/icon_512x512@2x.png "$DOC_ICON/icon_512x512@2x.png"

cp "$DOC_ICON/icon_32x32.png"   "$DOC_ICON/icon_16x16@2x.png"
cp "$DOC_ICON/icon_256x256.png" "$DOC_ICON/icon_128x128@2x.png"
cp "$DOC_ICON/icon_512x512.png" "$DOC_ICON/icon_256x256@2x.png"

rm 8.png
rm -rf template.iconset
