#!/usr/bin/env sh

# taken from https://github.com/Nheko-Reborn/nheko

# Runs the license update
# Return codes:
#  - 1 there are files to be formatted
#  - 0 everything looks fine

set -eu

FILES=$(find polyglot-cpp polyglot-d polybuild examples -type f \( -iname "*.cpp" -o -iname "*.h" \))
clang-format -i $FILES
git diff --exit-code
