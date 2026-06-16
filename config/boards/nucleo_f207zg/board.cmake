# Board target (BOARD=nucleo_f207zg).
# Module enable/disable: boards_cfg.h only (#if in C). CMake always links all driver modules.
set(MCU_NAME stm32f207zg)
set(FAMILY_NAME stm32f2xx)
set(DEVICE_DEFINE STM32F207xx)
set(CORE_TYPE cortex-m3)

set(BOARD_SOURCES
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/driver.c
  ${CMAKE_SOURCE_DIR}/drivers/hal_override.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/clk/clk.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/uart/uart.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/i2c/i2c.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/h_timer/h_timer.c
  ${CMAKE_SOURCE_DIR}/middleware/h_soft_timer/h_soft_timer.c
  ${CMAKE_SOURCE_DIR}/drivers/device/led/led.c
  ${CMAKE_SOURCE_DIR}/drivers/device/ir_rx/ir_rx_time.c
  ${CMAKE_SOURCE_DIR}/drivers/device/ir_rx/ir_rx_drv.c
  ${CMAKE_SOURCE_DIR}/drivers/device/ir_tx/ir_tx_drv.c
  ${CMAKE_SOURCE_DIR}/drivers/device/mpu6050/mpu6050.c
  ${CMAKE_SOURCE_DIR}/middleware/ir/ir_rx_nec.c
  ${CMAKE_SOURCE_DIR}/middleware/ir/ir_rx.c
  ${CMAKE_SOURCE_DIR}/middleware/ir/ir_tx_nec.c
  ${CMAKE_SOURCE_DIR}/middleware/ir/ir_tx.c
  ${CMAKE_SOURCE_DIR}/middleware/serial/serial.c
)

set(BOARD_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/drivers
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/clk
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/uart
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/i2c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32f2/h_timer
  ${CMAKE_SOURCE_DIR}/drivers/device/led
  ${CMAKE_SOURCE_DIR}/drivers/device/ir_rx
  ${CMAKE_SOURCE_DIR}/drivers/device/ir_tx
  ${CMAKE_SOURCE_DIR}/drivers/device/mpu6050
  ${CMAKE_SOURCE_DIR}/middleware/h_soft_timer
  ${CMAKE_SOURCE_DIR}/middleware/ir
  ${CMAKE_SOURCE_DIR}/middleware/serial
)
