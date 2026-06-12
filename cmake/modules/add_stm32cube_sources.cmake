function(add_stm32cube_sources target)
  if(NOT DEFINED FAMILY_NAME)
    message(FATAL_ERROR "FAMILY_NAME is not set (config/boards/<board>/board.cmake)")
  endif()
  if(NOT DEFINED FREERTOS_PORT)
    message(FATAL_ERROR "FREERTOS_PORT is not set (CORE_TYPE in board.cmake)")
  endif()

  set(HAL_DIR "${CMAKE_SOURCE_DIR}/third_party/stm32cube/hal/${FAMILY_NAME}")
  set(CMSIS_DIR "${CMAKE_SOURCE_DIR}/third_party/stm32cube/cmsis")
  set(CMSIS_DEVICE_DIR "${CMSIS_DIR}/device/st/${FAMILY_NAME}")
  set(HAL_PREFIX "${FAMILY_NAME}_hal")
  set(FREERTOS_DIR "${CMAKE_SOURCE_DIR}/third_party/stm32cube/freertos")
  set(FREERTOS_PORT_DIR "${FREERTOS_DIR}/portable/GCC/${FREERTOS_PORT}")

  target_include_directories(${target} PRIVATE
    ${CMSIS_DIR}/include
    ${CMSIS_DEVICE_DIR}/include
    ${HAL_DIR}/inc
    ${FREERTOS_DIR}/include
    ${FREERTOS_PORT_DIR}
  )

  set(STM32CUBE_HAL_SOURCES
    ${HAL_DIR}/src/${HAL_PREFIX}.c
    ${HAL_DIR}/src/${HAL_PREFIX}_dma.c
    ${HAL_DIR}/src/${HAL_PREFIX}_flash.c
    ${HAL_DIR}/src/${HAL_PREFIX}_flash_ex.c
    ${HAL_DIR}/src/${HAL_PREFIX}_gpio.c
    ${HAL_DIR}/src/${HAL_PREFIX}_rcc.c
    ${HAL_DIR}/src/${HAL_PREFIX}_cortex.c
    ${HAL_DIR}/src/${HAL_PREFIX}_uart.c
    ${HAL_DIR}/src/${HAL_PREFIX}_tim.c
    ${HAL_DIR}/src/${HAL_PREFIX}_tim_ex.c
    ${HAL_DIR}/src/${HAL_PREFIX}_i2c.c
    ${HAL_DIR}/src/${HAL_PREFIX}_exti.c
  )

  target_sources(${target} PRIVATE
    ${SYSTEM_FILE}
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_PORT_DIR}/port.c
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
    ${STM32CUBE_HAL_SOURCES}
    ${STARTUP_FILE}
  )

  # STM32Cube HAL: suppress vendor warnings; do not patch third_party sources.
  set_source_files_properties(${STM32CUBE_HAL_SOURCES}
    PROPERTIES COMPILE_OPTIONS "-Wno-unused-parameter")
endfunction()
