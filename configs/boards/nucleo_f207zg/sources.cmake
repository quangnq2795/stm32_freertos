# Board-specific BSP sources and include dirs (included from add_firmware_target).

target_sources(firmware.elf PRIVATE
  ${CMAKE_SOURCE_DIR}/driver/bsp/common/bsp.c
  ${CMAKE_SOURCE_DIR}/driver/bsp/device/led/${board_name}/led.c
)

target_include_directories(firmware.elf PRIVATE
  ${CMAKE_SOURCE_DIR}/driver/bsp/common
  ${CMAKE_SOURCE_DIR}/driver/bsp/device/led/${board_name}
)
