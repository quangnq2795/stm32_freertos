#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
LOCAL_CMAKE="${ROOT_DIR}/toolchains/cmake/cmake-3.30.5-linux-x86_64/bin/cmake"
BOARD_NAME="${BOARD:-nucleo_f207zg}"

usage() {
  cat <<EOF
Usage: $(basename "$0") [clean] [build]

  (no args)   Configure and build (default)
  clean       Remove ${BUILD_DIR}
  build       Configure and build
  clean build Full clean, then configure and build

Environment:
  BOARD       Board name (default: nucleo_f207zg)
EOF
}

if [[ -x "${LOCAL_CMAKE}" ]]; then
  CMAKE_BIN="${LOCAL_CMAKE}"
else
  CMAKE_BIN="cmake"
fi

DO_CLEAN=0
DO_BUILD=0

if [[ $# -eq 0 ]]; then
  DO_BUILD=1
else
  for arg in "$@"; do
    case "${arg}" in
      clean)
        DO_CLEAN=1
        ;;
      build)
        DO_BUILD=1
        ;;
      -h|--help)
        usage
        exit 0
        ;;
      *)
        echo "Unknown option: ${arg}" >&2
        usage >&2
        exit 1
        ;;
    esac
  done
fi

if [[ "${DO_CLEAN}" -eq 1 ]]; then
  echo "Cleaning ${BUILD_DIR}..."
  rm -rf "${BUILD_DIR}"
fi

if [[ "${DO_BUILD}" -eq 1 ]]; then
  "${CMAKE_BIN}" -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/cmake/toolchains/arm-none-eabi-gcc.cmake" \
    -DBOARD="${BOARD_NAME}" \
    -DCMAKE_BUILD_TYPE=Debug

  "${CMAKE_BIN}" --build "${BUILD_DIR}" -j
fi
