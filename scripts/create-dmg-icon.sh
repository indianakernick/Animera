#!/bin/sh

# Creates the document icon by compositing the app icon with the generic
# volume icon template.

# Requires PROJECT_DIR
# Requires gm (graphicsmagick)

APP_ICON="$PROJECT_DIR/Assets.xcassets/AppIcon.appiconset"

iconutil --convert iconset --output template.iconset /System/Library/Extensions/IOStorageFamily.kext/Contents/Resources/Removable.icns
gm convert -resize 8x8 "$APP_ICON/16.png" 8.png
mkdir VolumeIcon.iconset

gm composite -geometry 8x7+4+2!         8.png               template.iconset/icon_16x16.png      VolumeIcon.iconset/icon_16x16.png
gm composite -geometry 16x14+8+4!       "$APP_ICON/16.png"  template.iconset/icon_32x32.png      VolumeIcon.iconset/icon_32x32.png
gm composite -geometry 32x28+16+8!      "$APP_ICON/32.png"  template.iconset/icon_32x32@2x.png   VolumeIcon.iconset/icon_32x32@2x.png
gm composite -geometry 64x56+32+16!     "$APP_ICON/64.png"  template.iconset/icon_128x128.png    VolumeIcon.iconset/icon_128x128.png
gm composite -geometry 128x112+64+32!   "$APP_ICON/128.png" template.iconset/icon_256x256.png    VolumeIcon.iconset/icon_256x256.png
gm composite -geometry 256x224+128+64!  "$APP_ICON/256.png" template.iconset/icon_512x512.png    VolumeIcon.iconset/icon_512x512.png
gm composite -geometry 512x448+256+128! "$APP_ICON/512.png" template.iconset/icon_512x512@2x.png VolumeIcon.iconset/icon_512x512@2x.png

cp VolumeIcon.iconset/icon_32x32.png   VolumeIcon.iconset/icon_16x16@2x.png
cp VolumeIcon.iconset/icon_256x256.png VolumeIcon.iconset/icon_128x128@2x.png
cp VolumeIcon.iconset/icon_512x512.png VolumeIcon.iconset/icon_256x256@2x.png

iconutil --convert icns VolumeIcon.iconset

rm 8.png
rm -rf template.iconset
rm -rf VolumeIcon.iconset
