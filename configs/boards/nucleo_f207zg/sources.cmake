# Board-specific BSP sources and include dirs (included from add_firmware_target).

target_sources(firmware.elf PRIVATE
  ${CMAKE_SOURCE_DIR}/driver/bsp/bsp.c
  ${CMAKE_SOURCE_DIR}/driver/bsp/led/led.c
  ${CMAKE_SOURCE_DIR}/driver/bsp/uart/uart.c
)

target_include_directories(firmware.elf PRIVATE
  ${CMAKE_SOURCE_DIR}/driver/bsp
  ${CMAKE_SOURCE_DIR}/driver/bsp/led
  ${CMAKE_SOURCE_DIR}/driver/bsp/uart
)
