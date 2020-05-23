#!/bin/sh

# Puts the app package into a dmg file

# Requires PROJECT_DIR, BUILT_PRODUCTS_DIR, FULL_PRODUCT_NAME, 
# CODE_SIGN_IDENTITY, EXPANDED_CODE_SIGN_IDENTITY
# Requires appdmg
# Requires envsubst from gettext

cat "$PROJECT_DIR/scripts/dmg.json" | envsubst > expanded.json
appdmg expanded.json "$PROJECT_DIR/Animera.dmg"
rm expanded.json
