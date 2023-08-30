#!/usr/bin/env sh

# taken from https://github.com/Nheko-Reborn/nheko

# Runs the license update
# Return codes:
#  - 1 there are files to be formatted
#  - 0 everything looks fine

set -eu

FILES=$(find src tests -type f \( -iname "*.cpp" -o -iname "*.h" -o -iname "*.d" -o -iname "*.rs" \))

reuse annotate --exclude-year --copyright="Loren Burkholder" --license="GPL-3.0" $FILES

git diff --exit-code
