#!/bin/sh

# Puts the app package into a dmg file

# Requires PROJECT_DIR, BUILT_PRODUCTS_DIR, FULL_PRODUCE_NAME, 
# CODE_SIGN_IDENTITY, EXPANDED_CODE_SIGN_IDENTITY
# Requires appdmg

sed "s~\\\$BUILT_PRODUCTS_DIR~$BUILT_PRODUCTS_DIR~g;s~\\\$FULL_PRODUCT_NAME~$FULL_PRODUCT_NAME~g;s~\\\$CODE_SIGN_IDENTITY~$CODE_SIGN_IDENTITY~g;s~\\\$EXPANDED_CODE_SIGN_IDENTITY~$EXPANDED_CODE_SIGN_IDENTITY~g" "$PROJECT_DIR/scripts/dmg.json" > expanded.json
appdmg expanded.json "$PROJECT_DIR/Animera.dmg"
rm expanded.json
