#!/usr/bin/env bash
#
# Flash ONLY the application image to the external Octo-SPI NOR flash
# (MX66UW1G45G) of the STM32N6570-DK (dk32n65).
#
# Layout assumed (two-stage boot):
#     0x70000000  FSBL          <- programmed separately, NOT touched here
#     0x70100000  Application    <- this script writes/reads only this region
#
# The FSBL at 0x70000000 is what the Boot ROM authenticates and runs; the FSBL
# in turn loads this application. So here we only deal with the app image.
#
# Requires STM32CubeProgrammer (STM32_Programmer_CLI + the
# MX66UW1G45G_STM32N6570-DK.stldr external loader). Signing is optional and
# only needed if your FSBL authenticates the application (SIGN=1).
#
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

# ----------------------------------------------------------------------------
# Config (override via environment)
# ----------------------------------------------------------------------------
BOARD="${BOARD:-dk32n65}"
APP_ADDR="${APP_ADDR:-0x70100000}"                  # application offset in external NOR (NOT the FSBL base)
APP_BIN="${APP_BIN:-${BUILD_DIR}/firmware_${BOARD}.bin}"
SIGN="${SIGN:-0}"                                   # 1 = sign app before flashing (FSBL authenticates it)
SIGNED_BIN="${SIGNED_BIN:-${BUILD_DIR}/firmware_${BOARD}-trusted.bin}"
READ_SIZE="${READ_SIZE:-0x40000}"                   # dump size for 'read' (256 KB)
READ_OUT="${READ_OUT:-${BUILD_DIR}/app-dump.bin}"

# STM32CubeProgrammer location: set STM32CUBEPROG_DIR, or rely on PATH.
CUBE_DIR="${STM32CUBEPROG_DIR:-}"

usage() {
  cat <<EOF
Usage: $(basename "$0") <command>

Flashes ONLY the application at ${APP_ADDR} (FSBL at 0x70000000 is left intact).

Commands:
  write           Program application image to ${APP_ADDR}
  verify          Program + read-back compare (-v)
  read            Dump ${READ_SIZE} bytes from ${APP_ADDR} -> ${READ_OUT}
  erase           Erase the application region (sector erase from ${APP_ADDR})

Environment overrides:
  BOARD              Board name (default dk32n65) -> firmware_<board>.bin
  APP_ADDR           Application address (default 0x70100000)
  APP_BIN            Application binary (default build/firmware_<board>.bin)
  SIGN               1 to sign the app before flashing (default 0)
  STM32CUBEPROG_DIR  Path to STM32CubeProgrammer install (else use PATH)
  READ_SIZE          Bytes to read for 'read' (default 0x40000)
  READ_OUT           Output file for 'read' (default build/app-dump.bin)
EOF
}

# ----------------------------------------------------------------------------
# Tool discovery
# ----------------------------------------------------------------------------
find_tool() {
  local name="$1"
  if [[ -n "${CUBE_DIR}" && -x "${CUBE_DIR}/bin/${name}" ]]; then
    echo "${CUBE_DIR}/bin/${name}"; return 0
  fi
  if command -v "${name}" >/dev/null 2>&1; then
    command -v "${name}"; return 0
  fi
  local p
  for p in \
    /opt/st/stm32cubeprog*/bin/"${name}" \
    /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/"${name}" \
    "${HOME}"/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/"${name}"; do
    [[ -x ${p} ]] && { echo "${p}"; return 0; }
  done
  echo ""
}

find_loader() {
  local prog_dir="$1"
  local base="${prog_dir%/bin}"
  local cand
  for cand in \
    "${base}/bin/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr" \
    "${base}/ExternalLoader/MX66UW1G45G_STM32N6570-DK.stldr"; do
    [[ -f ${cand} ]] && { echo "${cand}"; return 0; }
  done
  find "${base}" -name 'MX66UW1G45G_STM32N6570-DK.stldr' 2>/dev/null | head -1
}

PROGRAMMER="$(find_tool STM32_Programmer_CLI)"
SIGNER="$(find_tool STM32_SigningTool_CLI)"

require_programmer() {
  if [[ -z "${PROGRAMMER}" ]]; then
    echo "ERROR: STM32_Programmer_CLI not found. Install STM32CubeProgrammer" >&2
    echo "       or set STM32CUBEPROG_DIR to its install path." >&2
    exit 1
  fi
}

LOADER=""
require_loader() {
  LOADER="$(find_loader "$(dirname "${PROGRAMMER}")")"
  if [[ -z "${LOADER}" || ! -f "${LOADER}" ]]; then
    echo "ERROR: MX66UW1G45G_STM32N6570-DK.stldr external loader not found." >&2
    echo "       It ships with STM32CubeProgrammer under bin/ExternalLoader/." >&2
    exit 1
  fi
}

# ----------------------------------------------------------------------------
# Resolve the image to flash (raw app, or signed app when SIGN=1)
# ----------------------------------------------------------------------------
resolve_image() {
  [[ -f "${APP_BIN}" ]] || { echo "ERROR: ${APP_BIN} not found. Build BOARD=${BOARD} first." >&2; exit 1; }
  if [[ "${SIGN}" != "1" ]]; then
    IMAGE="${APP_BIN}"
    return
  fi
  if [[ -z "${SIGNER}" ]]; then
    echo "ERROR: SIGN=1 but STM32_SigningTool_CLI not found." >&2
    exit 1
  fi
  if [[ ! -f "${SIGNED_BIN}" || "${APP_BIN}" -nt "${SIGNED_BIN}" ]]; then
    echo ">> Signing application ${APP_BIN} -> ${SIGNED_BIN}"
    # -t appli : image type = application (loaded by the FSBL, not the Boot ROM).
    # Adjust flags to match what YOUR FSBL expects to authenticate.
    "${SIGNER}" -bin "${APP_BIN}" -nk -t appli -hv 2.3 -o "${SIGNED_BIN}"
  fi
  IMAGE="${SIGNED_BIN}"
}

# ----------------------------------------------------------------------------
# Commands
# ----------------------------------------------------------------------------
IMAGE=""

do_write() {
  require_programmer; require_loader; resolve_image
  echo ">> Writing application ${IMAGE} to ${APP_ADDR} via $(basename "${LOADER}")"
  "${PROGRAMMER}" -c port=SWD mode=HOTPLUG ap=1 \
    -el "${LOADER}" -hardRst \
    -w "${IMAGE}" "${APP_ADDR}"
  echo ">> Done (FSBL at 0x70000000 untouched). Power-cycle to run."
}

do_verify() {
  require_programmer; require_loader; resolve_image
  echo ">> Writing + verifying application ${IMAGE} at ${APP_ADDR}"
  "${PROGRAMMER}" -c port=SWD mode=HOTPLUG ap=1 \
    -el "${LOADER}" -hardRst \
    -w "${IMAGE}" "${APP_ADDR}" -v
}

do_read() {
  require_programmer; require_loader
  echo ">> Reading ${READ_SIZE} bytes from ${APP_ADDR} -> ${READ_OUT}"
  "${PROGRAMMER}" -c port=SWD mode=HOTPLUG ap=1 \
    -el "${LOADER}" \
    -r "${APP_ADDR}" "${READ_SIZE}" "${READ_OUT}"
  echo ">> Saved: ${READ_OUT}"
}

do_erase() {
  require_programmer; require_loader
  echo ">> Erasing application region from ${APP_ADDR} (${READ_SIZE} bytes)"
  "${PROGRAMMER}" -c port=SWD mode=HOTPLUG ap=1 \
    -el "${LOADER}" \
    -e "[${APP_ADDR} ${READ_SIZE}]"
}

case "${1:-}" in
  write)  do_write  ;;
  verify) do_verify ;;
  read)   do_read   ;;
  erase)  do_erase  ;;
  -h|--help|"") usage ;;
  *) echo "Unknown command: $1" >&2; usage; exit 1 ;;
esac
