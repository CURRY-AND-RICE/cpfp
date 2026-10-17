#!/usr/bin/env bash
set -uo pipefail
cd "$(dirname "$0")/.."

failures=0
CPFP_CLANGD="${CLANGD:-clangd}"
CPFP_CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
CPFP_CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"

ok() {
    printf 'OK   %s\n' "$1"
}

warn() {
    printf 'WARN %s\n' "$1"
}

fail() {
    printf 'FAIL %s\n' "$1" >&2
    failures=$((failures + 1))
}

require_command() {
    local command_name="$1"
    if command -v "${command_name}" >/dev/null 2>&1; then
        ok "${command_name}: $(command -v "${command_name}")"
    else
        fail "${command_name} is not installed or not on PATH"
    fi
}

version_at_least() {
    local actual="$1"
    local minimum="$2"
    [[ "$(printf '%s\n%s\n' "${minimum}" "${actual}" | sort -V | head -n 1)" == "${minimum}" ]]
}

for command_name in cmake ninja g++ \
    "${CPFP_CLANGD}" "${CPFP_CLANG_FORMAT}" "${CPFP_CLANG_TIDY}"; do
    require_command "${command_name}"
done

for include_variable in CPATH CPLUS_INCLUDE_PATH C_INCLUDE_PATH OBJC_INCLUDE_PATH; do
    include_value="${!include_variable-}"
    if [[ -n "${include_value}" ]]; then
        ok "${include_variable} is set; scripts/check.sh ignores it while verifying cpfp"
    fi
done

if command -v cmake >/dev/null 2>&1; then
    cmake_version="$(cmake --version | awk 'NR == 1 { print $3 }')"
    if version_at_least "${cmake_version}" 3.25; then
        ok "CMake ${cmake_version} (required: 3.25 or newer)"
    else
        fail "CMake ${cmake_version} is too old; 3.25 or newer is required"
    fi
fi

if command -v g++ >/dev/null 2>&1; then
    gcc_version="$(g++ -dumpfullversion -dumpversion)"
    if [[ "${gcc_version}" == 15.2.* ]]; then
        ok "GCC ${gcc_version}"
    else
        fail "GCC 15.2.x is required; found ${gcc_version}"
    fi
fi

for llvm_tool in "${CPFP_CLANGD}" "${CPFP_CLANG_FORMAT}" "${CPFP_CLANG_TIDY}"; do
    if command -v "${llvm_tool}" >/dev/null 2>&1; then
        llvm_version="$(${llvm_tool} --version | head -n 1)"
        if [[ "${llvm_version}" == *"version 21."* ]]; then
            ok "${llvm_version}"
        else
            fail "${llvm_tool} is not LLVM 21: ${llvm_version}"
        fi
    fi
done

if [[ -z "${BOOST_ROOT:-}" ]]; then
    fail "BOOST_ROOT is not set"
else
    boost_version_header="${BOOST_ROOT}/boost/version.hpp"
    if [[ -f "${boost_version_header}" ]]; then
        boost_version="$(awk '/^#define BOOST_VERSION [0-9]+$/ { print $3 }' "${boost_version_header}")"
        if [[ "${boost_version}" == 108800 ]]; then
            ok "Boost 1.88.0: ${BOOST_ROOT}"
        else
            fail "Boost 1.88.0 is required; ${BOOST_ROOT} reports BOOST_VERSION=${boost_version:-unknown}"
        fi
    else
        fail "Boost headers not found under ${BOOST_ROOT}"
    fi
fi

if [[ -f build/debug/compile_commands.json ]]; then
    ok "clangd compilation database: build/debug/compile_commands.json"
else
    warn "build/debug/compile_commands.json is missing; run: cmake --fresh --preset gcc-debug"
fi

if command -v pre-commit >/dev/null 2>&1; then
    ok "optional pre-commit: $(command -v pre-commit)"
else
    warn "optional pre-commit is not installed"
fi

if ((failures > 0)); then
    printf '\nEnvironment check failed with %d error(s).\n' "${failures}" >&2
    exit 1
fi

printf '\nEnvironment check passed.\n'
