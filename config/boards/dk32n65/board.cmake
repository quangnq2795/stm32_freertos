# Board target (BOARD=dk32n65) — STM32N6570-DK / STM32N657X0H3Q.
set(MCU_NAME stm32n657x0h3q)
set(FAMILY_NAME stm32n6xx)
set(DEVICE_DEFINE STM32N657xx)
set(CORE_TYPE cortex-m55)
set(HAL_PREFIX stm32n6xx_hal)
set(CORE_MCU_DIR "${CMAKE_SOURCE_DIR}/core/${MCU_NAME}")

set(BOARD_SOURCES
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/driver.c
  ${CMAKE_SOURCE_DIR}/drivers/hal_override.c
  ${CMAKE_SOURCE_DIR}/third_party/stm32cube/bsp/stm32n6570-dk/stm32n6570_discovery.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/clk/clk.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/uart/uart.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/i2c/i2c.c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/h_timer/h_timer.c
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
  ${CORE_MCU_DIR}/support/syscalls.c
  ${CORE_MCU_DIR}/support/sysmem.c
)

set(BOARD_INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/drivers
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/clk
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/uart
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/i2c
  ${CMAKE_SOURCE_DIR}/drivers/platform/stm32n6/h_timer
  ${CMAKE_SOURCE_DIR}/drivers/device/led
  ${CMAKE_SOURCE_DIR}/drivers/device/ir_rx
  ${CMAKE_SOURCE_DIR}/drivers/device/ir_tx
  ${CMAKE_SOURCE_DIR}/drivers/device/mpu6050
  ${CMAKE_SOURCE_DIR}/middleware/h_soft_timer
  ${CMAKE_SOURCE_DIR}/middleware/ir
  ${CMAKE_SOURCE_DIR}/middleware/serial
  ${CMAKE_SOURCE_DIR}/third_party/stm32cube/bsp/stm32n6570-dk
)
