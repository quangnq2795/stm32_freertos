# STM32F2 FreeRTOS

Firmware build with **MCU** + **board** as two independent CMake options.

## Layout (short)

| Path | Role |
|------|------|
| `core/<mcu>/` | Startup, system, linker, **`firmware.cmake`** (toolchain, `DEVICE_DEFINE`, FreeRTOS port name) |
| `configs/hal/<mcu>/` | `stm32f2xx_hal_conf.h` |
| `configs/freertos/<mcu>/` | `FreeRTOSConfig.h` |
| `configs/boards/<board>/` | `bsp_clk_cfg.h` (and component cfg headers), **`sources.cmake`** |
| `driver/bsp/common/` | `bsp.c` / `bsp.h` |
| `driver/bsp/device/<component>/<board>/` | e.g. LED driver |
| `app/` | `main`, tasks |
| `driver/cmsis`, `driver/hal`, `middlewares/freertos` | CMSIS + STM32F2 HAL + FreeRTOS kernel + port |

**CMSIS device** for F2 must be under `driver/cmsis/device/st/stm32f2xx/include/` (lowercase **`include`**, not `Include`).

## Default: NUCLEO-F207ZG

- **MCU:** `stm32f207zg` — Cortex-M3, `STM32F207xx`, port `ARM_CM3`
- **Board:** `nucleo_f207zg` — user LED LD2 = **PB7**

## Build

```bash
bash scripts/build.sh
# override:
MCU_NAME=stm32f207zg BOARD=nucleo_f207zg bash scripts/build.sh
```

Artifacts: `build/firmware.elf`, `.hex`, `.bin`.

`vendor/stm32cube_f2` is a shallow STM32CubeF2 clone (reference for copying startup/HAL). See `docs/add-new-mcu-guide.md` to add another F2 MCU or board.
