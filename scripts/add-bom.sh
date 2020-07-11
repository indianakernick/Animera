#!/bin/sh

# Adds UTF-8 BOMs to all source files that don't have it. This is
# required for compilation on Windows. 

# Requires PROJECT_DIR
# Requires uconv

# https://stackoverflow.com/a/3293858

for file in "$PROJECT_DIR"/src/*; do
  # echo "# Processing: $file" 1>&2
  if [ ! -f "$file" ]; then
    echo Not a file: "$file" 1>&2
    exit 1
  fi
  TYPE=$(file - < "$file" | cut -d: -f2)
  if echo "$TYPE" | grep -q '(with BOM)'; then
    : # echo "# $file already has BOM, skipping." 1>&2
  else
    ( mv "${file}" "${file}"~ && /usr/local/opt/icu4c/bin/uconv -f utf-8 -t utf-8 --add-signature < "${file}~" > "${file}" ) || ( echo Error processing "$file" 1>&2 ; exit 1)
    rm "${file}~"
  fi
done
