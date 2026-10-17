#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
CPFP_CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
mapfile -d '' sources < <(find include tests examples -type f \
    \( -name '*.hpp' -o -name '*.cpp' \) -print0)
"${CPFP_CLANG_FORMAT}" -i "${sources[@]}"
