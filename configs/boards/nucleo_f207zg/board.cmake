# Board target (BOARD=nucleo_f207zg).
set(MCU_NAME stm32f207zg)
set(FAMILY_NAME stm32f2xx)
set(DEVICE_DEFINE STM32F207xx)
set(CORE_TYPE cortex-m3)

set(BOARD_SOURCES
  ${CMAKE_SOURCE_DIR}/driver/bsp/bsp.c
  ${CMAKE_SOURCE_DIR}/driver/bsp/clk/clk.c
  ${CMAKE_SOURCE_DIR}/driver/bsp/led/led.c
  ${CMAKE_SOURCE_DIR}/driver/bsp/uart/uart.c
)

set(BOARD_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/driver/bsp
  ${CMAKE_SOURCE_DIR}/driver/bsp/clk
  ${CMAKE_SOURCE_DIR}/driver/bsp/led
  ${CMAKE_SOURCE_DIR}/driver/bsp/uart
)
