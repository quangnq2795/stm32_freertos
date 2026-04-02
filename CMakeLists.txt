cmake_minimum_required(VERSION 3.16)
project(stm32f2_freertos C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT MCU_NAME)
  set(MCU_NAME stm32f207zg)
endif()
if(NOT BOARD)
  set(BOARD nucleo_f207zg)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/modules/add_firmware_target.cmake)
add_firmware_target(${BOARD})
