#!/usr/bin/env bash
set -euo pipefail

# Install ARM GNU Toolchain 13.2 to /opt/arm-gnu-toolchain-13.2 (required for all boards).
#
#   sudo bash scripts/install_arm_toolchain.sh
#   sudo bash scripts/install_arm_toolchain.sh --from /path/to/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi

VERSION="13.2.Rel1"
ARCH="x86_64-arm-none-eabi"
TARBALL="arm-gnu-toolchain-${VERSION}-${ARCH}.tar.xz"
URL="https://developer.arm.com/-/media/Files/downloads/gnu/${VERSION}/binrel/${TARBALL}"
INSTALL_PREFIX="/opt/arm-gnu-toolchain-13.2"
FROM_DIR=""
TMP_DIR=""

cleanup() {
  if [[ -n "${TMP_DIR}" ]]; then
    rm -rf "${TMP_DIR}"
  fi
}
trap cleanup EXIT

usage() {
  cat <<EOF
Usage: $(basename "$0") [--from <extracted-toolchain-dir>]

Install ARM GNU Toolchain ${VERSION} to ${INSTALL_PREFIX}.

Options:
  --from DIR   Use an existing extracted toolchain (skip download)
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --from)
      FROM_DIR="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

install_tree() {
  local src="$1"

  if [[ ! -x "${src}/bin/arm-none-eabi-gcc" ]]; then
    echo "Not a valid ARM toolchain: ${src}/bin/arm-none-eabi-gcc" >&2
    exit 1
  fi

  echo "Installing to ${INSTALL_PREFIX} ..."
  sudo rm -rf "${INSTALL_PREFIX}"
  sudo mv "${src}" "${INSTALL_PREFIX}"
}

if [[ -n "${FROM_DIR}" ]]; then
  if [[ ! -d "${FROM_DIR}" ]]; then
    echo "Directory not found: ${FROM_DIR}" >&2
    exit 1
  fi
  FROM_DIR="$(cd "${FROM_DIR}" && pwd)"
  echo "Installing from ${FROM_DIR} ..."
  install_tree "${FROM_DIR}"
else
  TMP_DIR="$(mktemp -d)"
  echo "Downloading ${URL} ..."
  curl -fsSL "${URL}" -o "${TMP_DIR}/${TARBALL}"

  echo "Extracting ..."
  tar -xJf "${TMP_DIR}/${TARBALL}" -C "${TMP_DIR}"

  EXTRACTED="$(find "${TMP_DIR}" -maxdepth 1 -type d -name 'arm-gnu-toolchain-*' | head -1)"
  if [[ -z "${EXTRACTED}" ]]; then
    echo "Could not find extracted toolchain directory." >&2
    exit 1
  fi
  install_tree "${EXTRACTED}"
fi

echo ""
echo "Installed: ${INSTALL_PREFIX}/bin/arm-none-eabi-gcc"
"${INSTALL_PREFIX}/bin/arm-none-eabi-gcc" --version | head -1
