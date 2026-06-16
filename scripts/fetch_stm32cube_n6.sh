#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CUBE_DIR="${STM32CUBE_N6_DIR:-/tmp/STM32CubeN6}"
FREERTOS_DIR="${FREERTOS_KERNEL_DIR:-/tmp/FreeRTOS-Kernel}"

usage() {
  cat <<EOF
Usage: $(basename "$0")

Fetch STM32N6 HAL/CMSIS/BSP and FreeRTOS CM55 port into third_party/.

Environment:
  STM32CUBE_N6_DIR   Path to STM32CubeN6 clone (default: /tmp/STM32CubeN6)
  FREERTOS_KERNEL_DIR Path to FreeRTOS-Kernel clone (default: /tmp/FreeRTOS-Kernel)
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ ! -d "${CUBE_DIR}/.git" ]]; then
  echo "STM32CubeN6 not found at ${CUBE_DIR}. Clone it first:" >&2
  echo "  git clone --depth 1 https://github.com/STMicroelectronics/STM32CubeN6.git ${CUBE_DIR}" >&2
  exit 1
fi

echo "Initializing STM32CubeN6 submodules (HAL, CMSIS device, DK BSP)..."
(
  cd "${CUBE_DIR}"
  git submodule update --init --depth 1 \
    Drivers/STM32N6xx_HAL_Driver \
    Drivers/CMSIS/Device/ST/STM32N6xx \
    Drivers/BSP/STM32N6570-DK
)

HAL_DST="${ROOT_DIR}/third_party/stm32cube/hal/stm32n6xx"
CMSIS_DST="${ROOT_DIR}/third_party/stm32cube/cmsis/device/st/stm32n6xx/include"
BSP_DST="${ROOT_DIR}/third_party/stm32cube/bsp/stm32n6570-dk"
FR_PORT_DST="${ROOT_DIR}/third_party/stm32cube/freertos/portable/GCC/ARM_CM55_NTZ"

echo "Syncing HAL -> ${HAL_DST}"
mkdir -p "${HAL_DST}/inc" "${HAL_DST}/src"
rsync -a --delete \
  "${CUBE_DIR}/Drivers/STM32N6xx_HAL_Driver/Inc/" "${HAL_DST}/inc/"
rsync -a --delete \
  "${CUBE_DIR}/Drivers/STM32N6xx_HAL_Driver/Src/" "${HAL_DST}/src/"

echo "Syncing CMSIS device -> ${CMSIS_DST}"
mkdir -p "${CMSIS_DST}"
rsync -a --delete \
  "${CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32N6xx/Include/" "${CMSIS_DST}/"

echo "Syncing STM32N6570-DK BSP -> ${BSP_DST}"
mkdir -p "${BSP_DST}"
rsync -a --delete \
  "${CUBE_DIR}/Drivers/BSP/STM32N6570-DK/" "${BSP_DST}/"

if [[ -d "${FREERTOS_DIR}/portable/GCC/ARM_CM55_NTZ" ]]; then
  echo "Syncing FreeRTOS ARM_CM55_NTZ port -> ${FR_PORT_DST}"
  mkdir -p "${FR_PORT_DST}"
  rsync -a --delete \
    "${FREERTOS_DIR}/portable/GCC/ARM_CM55_NTZ/" "${FR_PORT_DST}/"
else
  echo "Warning: FreeRTOS CM55 port not found at ${FREERTOS_DIR}" >&2
  echo "Clone: git clone --depth 1 https://github.com/FreeRTOS/FreeRTOS-Kernel.git ${FREERTOS_DIR}" >&2
fi

echo "Done."
