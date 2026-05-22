# Board target (BOARD=nucleo_f207zg).
# Module enable/disable: boards_cfg.h only (#if in C). CMake always links all driver modules.
set(MCU_NAME stm32f207zg)
set(FAMILY_NAME stm32f2xx)
set(DEVICE_DEFINE STM32F207xx)
set(CORE_TYPE cortex-m3)

set(BOARD_SOURCES
  ${CMAKE_SOURCE_DIR}/driver/driver.c
  ${CMAKE_SOURCE_DIR}/driver/hal_override.c
  ${CMAKE_SOURCE_DIR}/driver/platform/clk/clk.c
  ${CMAKE_SOURCE_DIR}/driver/platform/uart/uart.c
  ${CMAKE_SOURCE_DIR}/driver/boards/led/led.c
)

set(BOARD_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/driver
  ${CMAKE_SOURCE_DIR}/driver/platform/clk
  ${CMAKE_SOURCE_DIR}/driver/platform/uart
  ${CMAKE_SOURCE_DIR}/driver/boards/led
)
