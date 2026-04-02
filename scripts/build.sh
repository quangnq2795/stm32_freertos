#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
LOCAL_CMAKE="${ROOT_DIR}/toolchains/cmake/cmake-3.30.5-linux-x86_64/bin/cmake"
BOARD_NAME="${BOARD:-nucleo_f207zg}"
MCU_NAME_VAR="${MCU_NAME:-stm32f207zg}"

if [[ -x "${LOCAL_CMAKE}" ]]; then
  CMAKE_BIN="${LOCAL_CMAKE}"
else
  CMAKE_BIN="cmake"
fi

"${CMAKE_BIN}" -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_TOOLCHAIN_FILE="${ROOT_DIR}/cmake/toolchains/arm-none-eabi-gcc.cmake" \
  -DMCU_NAME="${MCU_NAME_VAR}" \
  -DBOARD="${BOARD_NAME}" \
  -DCMAKE_BUILD_TYPE=Debug

"${CMAKE_BIN}" --build "${BUILD_DIR}" -j
