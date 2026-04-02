# Guide: Add a new STM32F2 MCU or board (manual)

This tree is **STM32F2-only** (HAL `stm32f2xx`, typical Cortex-M3, FreeRTOS `ARM_CM3`).

Layout:

- `core/<mcu>/...` (startup, system, linker, **`firmware.cmake`**)
- `configs/boards/<board>/...` (e.g. `bsp_clk_cfg.h` + component cfg headers, **`sources.cmake`**)
- `driver/bsp/common/...`, `driver/bsp/device/<component>/<board>/...`
- `driver/cmsis/device/st/stm32f2xx/include/` — device headers (**folder name `include`**)
- `driver/hal/stm32f2xx/{inc,src}` — shared F2 HAL
- `configs/hal/<mcu>/stm32f2xx_hal_conf.h`, `configs/freertos/<mcu>/FreeRTOSConfig.h`

Example MCU name in this guide: `stm32f205rg` (adjust to your part).

## 1) Third-party sources

```bash
git clone --depth 1 https://github.com/STMicroelectronics/STM32CubeF2.git vendor/stm32cube_f2
```

## 2) Core folder

```bash
mkdir -p core/stm32f205rg/{startup,system,linker}
```

## 3) Startup / system / linker

Pick files that match **exact** sub-family and package (flash/RAM).

```bash
V=vendor/stm32cube_f2
cp "$V/Drivers/CMSIS/Device/ST/STM32F2xx/Source/Templates/gcc/startup_stm32f205xx.s" core/stm32f205rg/startup/
cp "$V/Projects/<YourBoard>/Templates/Src/system_stm32f2xx.c" core/stm32f205rg/system/
cp "$V/Projects/<YourBoard>/Templates/SW4STM32/.../STM32F205RGTx_FLASH.ld" core/stm32f205rg/linker/
```

## 4) HAL + CMSIS device (if not already in tree)

```bash
mkdir -p driver/cmsis/device/st/stm32f2xx/include
mkdir -p driver/hal/stm32f2xx/inc driver/hal/stm32f2xx/src
cp -r "$V/Drivers/CMSIS/Include/." driver/cmsis/include/
mkdir -p driver/cmsis/device/st/stm32f2xx
cp -r "$V/Drivers/CMSIS/Device/ST/STM32F2xx/Include" driver/cmsis/device/st/stm32f2xx/include
cp -r "$V/Drivers/STM32F2xx_HAL_Driver/Inc/." driver/hal/stm32f2xx/inc/
cp -r "$V/Drivers/STM32F2xx_HAL_Driver/Src/." driver/hal/stm32f2xx/src/
```

## 5) `core/<mcu>/firmware.cmake`

Example for another Cortex-M3 F2 part:

```cmake
set(MCU_FLAGS -mcpu=cortex-m3 -mthumb)
set(FAMILY_NAME stm32f2xx)
set(DEVICE_DEFINE STM32F205xx)
set(FREERTOS_PORT ARM_CM3)
set(CORE_MCU_DIR "${CMAKE_SOURCE_DIR}/core/${MCU_NAME}")
set(LINKER_SCRIPT "${CORE_MCU_DIR}/linker/STM32F205RGTx_FLASH.ld")
set(STARTUP_FILE "${CORE_MCU_DIR}/startup/startup_stm32f205xx.s")
set(SYSTEM_FILE "${CORE_MCU_DIR}/system/system_stm32f2xx.c")
```

`DEVICE_DEFINE` must match the line defined in `stm32f2xx.h` (`STM32F205xx`, `STM32F207xx`, …).

## 6) HAL + FreeRTOS config

```bash
mkdir -p configs/hal/stm32f205rg configs/freertos/stm32f205rg
cp configs/hal/stm32f207zg/stm32f2xx_hal_conf.h configs/hal/stm32f205rg/
cp configs/freertos/stm32f207zg/FreeRTOSConfig.h configs/freertos/stm32f205rg/
```

Edit `stm32f2xx_hal_conf.h` and `FreeRTOSConfig.h` for clocks, heap, and priorities.

## 7) New board

```bash
mkdir -p configs/boards/my_board driver/bsp/device/led/my_board
```

Add `bsp_clk_cfg.h` (clock/PLL + `stm32f2xx_hal.h`), `bsp_led_cfg.h` / `bsp_uart_cfg.h` as needed, `sources.cmake` (mirror `configs/boards/nucleo_f207zg/sources.cmake`).

## 8) Build

```bash
MCU_NAME=stm32f205rg BOARD=my_board bash scripts/build.sh
```

## 9) Checklist

- Configure succeeds; `firmware.elf` links.
- `Reset_Handler` present; scheduler runs; LED task works on hardware.

## Notes

- FreeRTOS port for this project is **`middlewares/freertos/portable/GCC/ARM_CM3`** (from Cube F2 if you refresh it).
- Adding a **non-F2** family requires new HAL/CMSIS trees and changes in `add_stm32cube_sources.cmake` (source file set differs per family).
