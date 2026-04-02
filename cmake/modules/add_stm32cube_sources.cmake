function(add_stm32cube_sources target)
  if(NOT DEFINED FAMILY_NAME)
    message(FATAL_ERROR "FAMILY_NAME is not set in core/${MCU_NAME}/firmware.cmake")
  endif()
  if(NOT DEFINED FREERTOS_PORT)
    message(FATAL_ERROR "FREERTOS_PORT is not set in core/${MCU_NAME}/firmware.cmake")
  endif()

  set(HAL_DIR "${CMAKE_SOURCE_DIR}/driver/hal/${FAMILY_NAME}")
  set(CMSIS_DEVICE_DIR "${CMAKE_SOURCE_DIR}/driver/cmsis/device/st/${FAMILY_NAME}")
  set(HAL_PREFIX "${FAMILY_NAME}_hal")
  set(FREERTOS_PORT_DIR "${CMAKE_SOURCE_DIR}/middlewares/freertos/portable/GCC/${FREERTOS_PORT}")

  target_include_directories(${target} PRIVATE
    ${CMAKE_SOURCE_DIR}/driver/cmsis/include
    ${CMSIS_DEVICE_DIR}/include
    ${HAL_DIR}/inc
    ${CMAKE_SOURCE_DIR}/middlewares/freertos/include
    ${FREERTOS_PORT_DIR}
  )

  target_sources(${target} PRIVATE
    ${SYSTEM_FILE}
    ${CMAKE_SOURCE_DIR}/middlewares/freertos/list.c
    ${CMAKE_SOURCE_DIR}/middlewares/freertos/queue.c
    ${CMAKE_SOURCE_DIR}/middlewares/freertos/tasks.c
    ${FREERTOS_PORT_DIR}/port.c
    ${CMAKE_SOURCE_DIR}/middlewares/freertos/portable/MemMang/heap_4.c
    ${HAL_DIR}/src/${HAL_PREFIX}.c
    ${HAL_DIR}/src/${HAL_PREFIX}_dma.c
    ${HAL_DIR}/src/${HAL_PREFIX}_flash.c
    ${HAL_DIR}/src/${HAL_PREFIX}_flash_ex.c
    ${HAL_DIR}/src/${HAL_PREFIX}_gpio.c
    ${HAL_DIR}/src/${HAL_PREFIX}_rcc.c
    ${HAL_DIR}/src/${HAL_PREFIX}_cortex.c
    ${HAL_DIR}/src/${HAL_PREFIX}_uart.c
    ${STARTUP_FILE}
  )
endfunction()
