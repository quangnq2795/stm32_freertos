include(${CMAKE_SOURCE_DIR}/cmake/modules/add_stm32cube_sources.cmake)

set(FIRMWARE_TARGET firmware.elf)

set(FIRMWARE_APP_SOURCES
  app/main.c
  app/tasks/os.c
  app/tasks/cli/task_cli.c
  app/tasks/ir/task_ir.c
  app/tasks/log/task_log.c
  app/tasks/sensor/task_sensor.c
  middleware/taskmanager/taskmanager.c
  middleware/serial/serial.c
  middleware/cli/cli.c
  middleware/ir/ir.c
  middleware/log/log.c
  middleware/sensor/sensor.c
  middleware/sensor/sensor_mpu6050.c
  lib/ringbuf/ringbuf.c
  lib/slot_queue/slot_queue.c
)

# Macros: include board/mcu cmake in caller scope (sets MCU_NAME, MCU_FLAGS, ...).
macro(_firmware_configure_board board_name)
  set(_board_cmake "${CMAKE_SOURCE_DIR}/config/boards/${board_name}/board.cmake")
  if(NOT EXISTS "${_board_cmake}")
    message(FATAL_ERROR "Board definition not found: ${_board_cmake}")
  endif()
  include("${_board_cmake}")

  foreach(_var IN ITEMS MCU_NAME FAMILY_NAME DEVICE_DEFINE CORE_TYPE)
    if(NOT DEFINED ${_var})
      message(FATAL_ERROR "${_var} is not set in ${_board_cmake}")
    endif()
  endforeach()

  if(CORE_TYPE STREQUAL "cortex-m0")
    set(MCU_FLAGS -mcpu=cortex-m0 -mthumb)
    set(FREERTOS_PORT ARM_CM0)
  elseif(CORE_TYPE STREQUAL "cortex-m0plus")
    set(MCU_FLAGS -mcpu=cortex-m0plus -mthumb)
    set(FREERTOS_PORT ARM_CM0)
  elseif(CORE_TYPE STREQUAL "cortex-m3")
    set(MCU_FLAGS -mcpu=cortex-m3 -mthumb)
    set(FREERTOS_PORT ARM_CM3)
  elseif(CORE_TYPE STREQUAL "cortex-m4")
    set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
    set(FREERTOS_PORT ARM_CM4F)
  elseif(CORE_TYPE STREQUAL "cortex-m7")
    set(MCU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)
    set(FREERTOS_PORT ARM_CM7)
  elseif(CORE_TYPE STREQUAL "cortex-m55")
    set(MCU_FLAGS -mcpu=cortex-m55 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -mcmse)
    set(FREERTOS_PORT ARM_CM55_NTZ/non_secure)
  else()
    message(FATAL_ERROR "Unknown CORE_TYPE '${CORE_TYPE}' in ${_board_cmake}")
  endif()

  include(${CMAKE_SOURCE_DIR}/core/${MCU_NAME}/firmware.cmake)
endmacro()

function(_firmware_apply_board target)
  if(BOARD_SOURCES)
    target_sources(${target} PRIVATE ${BOARD_SOURCES})
  endif()
  if(BOARD_INCLUDE_DIRS)
    target_include_directories(${target} PRIVATE ${BOARD_INCLUDE_DIRS})
  endif()
endfunction()

function(_firmware_apply_toolchain target basename)
  target_compile_options(${target} PRIVATE
    ${MCU_FLAGS}
    -ffunction-sections -fdata-sections -Wall -Wextra
  )
  target_link_options(${target} PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/${basename}.map
    --specs=nano.specs
  )
endfunction()

function(_firmware_apply_app_includes target board_name)
  target_include_directories(${target} PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/app/tasks/cli
    ${CMAKE_SOURCE_DIR}/app/tasks/ir
    ${CMAKE_SOURCE_DIR}/app/tasks/log
    ${CMAKE_SOURCE_DIR}/app/tasks/sensor
    ${CMAKE_SOURCE_DIR}/middleware
    ${CMAKE_SOURCE_DIR}/middleware/taskmanager
    ${CMAKE_SOURCE_DIR}/middleware/serial
    ${CMAKE_SOURCE_DIR}/middleware/cli
    ${CMAKE_SOURCE_DIR}/middleware/ir
    ${CMAKE_SOURCE_DIR}/middleware/log
    ${CMAKE_SOURCE_DIR}/middleware/sensor
    ${CMAKE_SOURCE_DIR}/middleware/h_soft_timer
    ${CMAKE_SOURCE_DIR}/drivers/device/ir_rx
    ${CMAKE_SOURCE_DIR}/drivers/device/ir_tx
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/config/app/${board_name}
    ${CMAKE_SOURCE_DIR}/config/app
    ${CMAKE_SOURCE_DIR}/config/boards/${board_name}
    ${CMAKE_SOURCE_DIR}/config/hal/${MCU_NAME}
    ${CMAKE_SOURCE_DIR}/config/freertos/${MCU_NAME}
    ${CMAKE_SOURCE_DIR}/bsp/${board_name}
    ${CMAKE_SOURCE_DIR}/lib/ringbuf
    ${CMAKE_SOURCE_DIR}/lib/slot_queue
  )
endfunction()

function(_firmware_add_hex_and_size target basename)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${target}> ${CMAKE_BINARY_DIR}/${basename}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target}> ${CMAKE_BINARY_DIR}/${basename}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target}>
  )
endfunction()

function(add_firmware_target board_name)
  _firmware_configure_board(${board_name})

  set(_basename "firmware_${board_name}")

  add_executable(${FIRMWARE_TARGET} ${FIRMWARE_APP_SOURCES})
  set_target_properties(${FIRMWARE_TARGET} PROPERTIES
    OUTPUT_NAME "${_basename}"
    SUFFIX ".elf"
  )

  _firmware_apply_board(${FIRMWARE_TARGET})
  _firmware_apply_toolchain(${FIRMWARE_TARGET} ${_basename})
  _firmware_apply_app_includes(${FIRMWARE_TARGET} ${board_name})

  target_compile_definitions(${FIRMWARE_TARGET} PRIVATE USE_HAL_DRIVER ${DEVICE_DEFINE})

  add_stm32cube_sources(${FIRMWARE_TARGET})
  _firmware_add_hex_and_size(${FIRMWARE_TARGET} ${_basename})
endfunction()
