# STM32 FreeRTOS

Firmware build is selected by **BOARD**; each board has a `board.cmake` with MCU/family/device/core type.

**Prerequisite:** [ARM GNU Toolchain 13.2](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) at `/opt/arm-gnu-toolchain-13.2` (all boards, including F2 and N6):

```bash
sudo bash scripts/install_arm_toolchain.sh
```

## Layout (short)

| Path | Role |
|------|------|
| `core/cmsis/` | ARM CMSIS core headers + ST device headers (`device/st/<family>/`) |
| `core/mcu/<mcu>/` | Startup, system, linker, **`firmware.cmake`** (paths only) |
| `configs/boards/<board>/board.cmake` | `MCU_NAME`, `FAMILY_NAME`, `DEVICE_DEFINE`, `CORE_TYPE`, `BOARD_SOURCES`, `BOARD_INCLUDE_DIRS` |
| `configs/hal/<mcu>/` | `stm32f2xx_hal_conf.h` |
| `configs/freertos/<mcu>/` | `FreeRTOSConfig.h` |
| `configs/boards/<board>/` | **`board.cmake`** (sources), **`boards_cfg.h`** (`CLK/UART/LED_ENABLED` for `#if` in C), `boards_*_cfg.h` |
| `driver/platform/` | MCU-facing drivers (clk, uart, …) |
| `driver/boards/` | Board wiring (led, …) |
| `driver/driver.c` | `driver_init()` — HAL; calls clk only if `CLK_ENABLED` in `boards_cfg.h` |
| `app/` | `main`, tasks |
| `driver/hal`, `middlewares/freertos` | STM32F2 HAL + FreeRTOS kernel + port |

**CMSIS device** for F2 must be under `core/cmsis/device/st/stm32f2xx/include/` (lowercase **`include`**, not `Include`).

## Default: NUCLEO-F207ZG

- **Board:** `nucleo_f207zg` — see `config/boards/nucleo_f207zg/board.cmake` (MCU `stm32f207zg`, `STM32F207xx`, `cortex-m3`)
- User LED LD2 = **PB7**

## STM32N6570-DK (dk32n65)

- **Board:** `dk32n65` — STM32N657X0H3Q on STM32N6570-DK (`cortex-m55`, 800 MHz)
- ST-Link VCP: **USART1** PE5/PE6 (115200, odd parity)
- LEDs: green **PO1**, red **PG10**
- Boot in **development mode** (BOOT1 = 1-3) to load/debug from RAM via ST-Link

Requires `scripts/fetch_stm32cube_n6.sh` before first build.

```bash
scripts/fetch_stm32cube_n6.sh
BOARD=dk32n65 bash scripts/build.sh
```

## Build

```bash
bash scripts/build.sh
# another board:
BOARD=dk32n65 bash scripts/build.sh
```

Artifacts: `build/firmware.elf`, `.hex`, `.bin`.

To add a board: create `configs/boards/<board>/board.cmake` (defines + `BOARD_SOURCES` / `BOARD_INCLUDE_DIRS`) and BSP cfg headers, plus `core/mcu/<mcu>/` and `configs/hal|freertos/<mcu>/` if needed.
