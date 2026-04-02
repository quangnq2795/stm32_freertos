include(${CMAKE_SOURCE_DIR}/core/${MCU_NAME}/firmware.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/modules/add_stm32cube_sources.cmake)

function(add_firmware_target board_name)
  if(NOT DEFINED DEVICE_DEFINE)
    message(FATAL_ERROR "DEVICE_DEFINE is not set in core/${MCU_NAME}/firmware.cmake")
  endif()
  if(NOT DEFINED FAMILY_NAME)
    message(FATAL_ERROR "FAMILY_NAME is not set in core/${MCU_NAME}/firmware.cmake")
  endif()
  if(NOT DEFINED MCU_NAME)
    message(FATAL_ERROR "MCU_NAME is not set before including firmware.cmake")
  endif()

  add_executable(firmware.elf
    app/main.c
    app/tasks/os.c
    app/tasks/cli/task_cli.c
    app/tasks/cli/handlers/cli_cmd_led.c
    app/tasks/cli/handlers/cli_cmd_sensor.c
    common/lib/ringbuf.c
  )

  include(${CMAKE_SOURCE_DIR}/configs/boards/${board_name}/sources.cmake)

  target_compile_options(firmware.elf PRIVATE
    ${MCU_FLAGS}
    -ffunction-sections -fdata-sections -Wall -Wextra
  )

  target_link_options(firmware.elf PRIVATE
    ${MCU_FLAGS}
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map
    --specs=nano.specs
  )

  target_include_directories(firmware.elf PRIVATE
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/app/tasks/cli
    ${CMAKE_SOURCE_DIR}/app/tasks/cli/handlers
    ${CMAKE_SOURCE_DIR}/configs
    ${CMAKE_SOURCE_DIR}/configs/boards/${board_name}
    ${CMAKE_SOURCE_DIR}/configs/hal/${MCU_NAME}
    ${CMAKE_SOURCE_DIR}/configs/freertos/${MCU_NAME}
    ${CMAKE_SOURCE_DIR}/common/lib
  )

  target_compile_definitions(firmware.elf PRIVATE USE_HAL_DRIVER ${DEVICE_DEFINE})

  add_stm32cube_sources(firmware.elf)

  add_custom_command(TARGET firmware.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:firmware.elf> ${CMAKE_BINARY_DIR}/firmware.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:firmware.elf> ${CMAKE_BINARY_DIR}/firmware.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware.elf>
  )
endfunction()
