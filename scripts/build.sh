#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
CMAKE_BIN="${CMAKE:-cmake}"

# Must match cmake/toolchains/arm-none-eabi-gcc.cmake
ARM_GNU_TOOLCHAIN_ROOT="/opt/arm-gnu-toolchain-13.2"

usage() {
  cat <<EOF
Usage: $(basename "$0") [clean] [build]

  (no args)   Interactive board menu
  clean       Remove ${BUILD_DIR}
  build       Configure and build (BOARD env or nucleo_f207zg)
  clean build Full clean, then configure and build

Requires ARM GNU Toolchain 13.2 at ${ARM_GNU_TOOLCHAIN_ROOT}
  sudo bash scripts/install_arm_toolchain.sh

Environment:
  BOARD       Board name (non-interactive: build / clean build)
  CMAKE       CMake binary (default: cmake from PATH)
EOF
}

require_arm_toolchain() {
  local gcc="${ARM_GNU_TOOLCHAIN_ROOT}/bin/arm-none-eabi-gcc"

  if [[ ! -x "${gcc}" ]]; then
    echo "ARM GNU Toolchain 13.2 not found at ${ARM_GNU_TOOLCHAIN_ROOT}." >&2
    echo "Install: sudo bash scripts/install_arm_toolchain.sh" >&2
    exit 1
  fi
}

do_clean() {
  echo "Cleaning ${BUILD_DIR}..."
  rm -rf "${BUILD_DIR}"
}

do_build() {
  local board_name="$1"

  require_arm_toolchain
  echo "Building board: ${board_name}"
  "${CMAKE_BIN}" -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/cmake/toolchains/arm-none-eabi-gcc.cmake" \
    -DBOARD="${board_name}" \
    -DCMAKE_BUILD_TYPE=Debug

  "${CMAKE_BIN}" --build "${BUILD_DIR}" -j
}

show_menu() {
  cat <<EOF

Select board:
  1) nucleo_f207zg   (STM32F207, NUCLEO-F207ZG)
  2) dk32n65         (STM32N6570-DK)
  9) clean           (remove build/)
  q) quit

EOF
}

interactive_menu() {
  local choice=""

  while true; do
    show_menu
    read -r -p "Choice: " choice

    case "${choice}" in
      1)
        do_build "nucleo_f207zg"
        return 0
        ;;
      2)
        do_build "dk32n65"
        return 0
        ;;
      9)
        do_clean
        echo "Done."
        ;;
      q|Q)
        return 0
        ;;
      *)
        echo "Invalid choice: ${choice}" >&2
        ;;
    esac
  done
}

DO_CLEAN=0
DO_BUILD=0
BOARD_NAME="${BOARD:-nucleo_f207zg}"

if [[ $# -eq 0 ]]; then
  if [[ -t 0 ]]; then
    interactive_menu
    exit 0
  fi
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
  do_clean
fi

if [[ "${DO_BUILD}" -eq 1 ]]; then
  do_build "${BOARD_NAME}"
fi
