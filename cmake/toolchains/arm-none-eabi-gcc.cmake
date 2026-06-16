set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(ARM_GNU_TOOLCHAIN_ROOT "/opt/arm-gnu-toolchain-13.2")
set(_ARM_TOOLCHAIN_BIN "${ARM_GNU_TOOLCHAIN_ROOT}/bin")

if(NOT EXISTS "${_ARM_TOOLCHAIN_BIN}/arm-none-eabi-gcc")
  message(FATAL_ERROR
    "ARM GNU Toolchain 13.2 not found at ${ARM_GNU_TOOLCHAIN_ROOT}.\n"
    "Install: sudo bash scripts/install_arm_toolchain.sh")
endif()

set(CMAKE_C_COMPILER "${_ARM_TOOLCHAIN_BIN}/arm-none-eabi-gcc")
set(CMAKE_ASM_COMPILER "${_ARM_TOOLCHAIN_BIN}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${_ARM_TOOLCHAIN_BIN}/arm-none-eabi-g++")
set(CMAKE_OBJCOPY "${_ARM_TOOLCHAIN_BIN}/arm-none-eabi-objcopy")
set(CMAKE_SIZE "${_ARM_TOOLCHAIN_BIN}/arm-none-eabi-size")

message(STATUS "ARM toolchain: ${_ARM_TOOLCHAIN_BIN}")
