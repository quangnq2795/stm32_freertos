# Board target (BOARD=nucleo_f207zg).
# Module enable/disable: boards_cfg.h only (#if in C). CMake always links all driver modules.
set(MCU_NAME stm32f207zg)
set(FAMILY_NAME stm32f2xx)
set(DEVICE_DEFINE STM32F207xx)
set(CORE_TYPE cortex-m3)

set(BOARD_SOURCES
  ${CMAKE_SOURCE_DIR}/drivers/driver.c
  ${CMAKE_SOURCE_DIR}/drivers/hal_override.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/clk/clk.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/uart/uart.c
  ${CMAKE_SOURCE_DIR}/drivers/device/led/led.c
)

set(BOARD_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/drivers
  ${CMAKE_SOURCE_DIR}/drivers/platform/clk
  ${CMAKE_SOURCE_DIR}/drivers/platform/uart
  ${CMAKE_SOURCE_DIR}/drivers/device/led
)
