#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

CPFP_CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
CPFP_CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"

# Do not let undeclared, machine-wide include paths hide missing dependencies.
unset CPATH CPLUS_INCLUDE_PATH C_INCLUDE_PATH OBJC_INCLUDE_PATH

if [[ -z "${BOOST_ROOT:-}" ]]; then
    echo "BOOST_ROOT is not set." >&2
    echo "Set it to the Boost 1.88.0 root directory." >&2
    exit 1
fi
CPFP_BOOST_ROOT="${BOOST_ROOT}"

if [[ ! -f "${CPFP_BOOST_ROOT}/boost/version.hpp" ]]; then
    echo "Boost headers not found under ${CPFP_BOOST_ROOT}" >&2
    echo "Set BOOST_ROOT to the Boost 1.88.0 root directory." >&2
    exit 1
fi
command -v "${CPFP_CLANG_FORMAT}" >/dev/null
command -v "${CPFP_CLANG_TIDY}" >/dev/null
export BOOST_ROOT="${CPFP_BOOST_ROOT}"

for preset in gcc-debug gcc-release gcc-sanitize; do
    # Avoid reusing a cached path to another Boost installation.
    cmake --fresh --preset "$preset"
    cmake --build --preset "$preset"
    ctest --preset "$preset"
done

mapfile -d '' sources < <(find include tests examples -type f \
    \( -name '*.hpp' -o -name '*.cpp' \) -print0)
"${CPFP_CLANG_FORMAT}" --dry-run --Werror "${sources[@]}"

# compile_commands.json also powers clangd.
"${CPFP_CLANG_TIDY}" -p build/debug \
    tests/smoke.cpp tests/header_self_contained.cpp examples/ex1.cpp

cmake --install build/release --prefix build/install
cmake --fresh -S tests/install_consumer -B build/install-consumer -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${PWD}/build/install"
cmake --build build/install-consumer
./build/install-consumer/cpfp_install_consumer
