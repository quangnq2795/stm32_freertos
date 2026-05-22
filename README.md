# STM32F2 FreeRTOS

Firmware build is selected by **BOARD**; each board has a `board.cmake` with MCU/family/device/core type.

## Layout (short)

| Path | Role |
|------|------|
| `core/cmsis/` | ARM CMSIS core headers + ST device headers (`device/st/<family>/`) |
| `core/mcu/<mcu>/` | Startup, system, linker, **`firmware.cmake`** (paths only) |
| `configs/boards/<board>/board.cmake` | `MCU_NAME`, `FAMILY_NAME`, `DEVICE_DEFINE`, `CORE_TYPE`, `BOARD_SOURCES`, `BOARD_INCLUDE_DIRS` |
| `configs/hal/<mcu>/` | `stm32f2xx_hal_conf.h` |
| `configs/freertos/<mcu>/` | `FreeRTOSConfig.h` |
| `configs/boards/<board>/` | **`board.cmake`**, `bsp_*_cfg.h` |
| `driver/bsp/common/` | `bsp.c` / `bsp.h` |
| `driver/bsp/device/<component>/<board>/` | e.g. LED driver |
| `app/` | `main`, tasks |
| `driver/hal`, `middlewares/freertos` | STM32F2 HAL + FreeRTOS kernel + port |

**CMSIS device** for F2 must be under `core/cmsis/device/st/stm32f2xx/include/` (lowercase **`include`**, not `Include`).

## Default: NUCLEO-F207ZG

- **Board:** `nucleo_f207zg` — see `configs/boards/nucleo_f207zg/board.cmake` (MCU `stm32f207zg`, `STM32F207xx`, `cortex-m3`)
- User LED LD2 = **PB7**

## Build

```bash
bash scripts/build.sh
# another board:
BOARD=my_board bash scripts/build.sh
```

Artifacts: `build/firmware.elf`, `.hex`, `.bin`.

To add a board: create `configs/boards/<board>/board.cmake` (defines + `BOARD_SOURCES` / `BOARD_INCLUDE_DIRS`) and BSP cfg headers, plus `core/mcu/<mcu>/` and `configs/hal|freertos/<mcu>/` if needed.
