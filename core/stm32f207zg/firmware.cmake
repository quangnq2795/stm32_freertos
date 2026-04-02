set(MCU_FLAGS
  -mcpu=cortex-m3
  -mthumb
)

set(FAMILY_NAME stm32f2xx)
set(DEVICE_DEFINE STM32F207xx)
set(FREERTOS_PORT ARM_CM3)
set(CORE_MCU_DIR "${CMAKE_SOURCE_DIR}/core/${MCU_NAME}")
set(LINKER_SCRIPT "${CORE_MCU_DIR}/linker/STM32F207ZGTx_FLASH.ld")
set(STARTUP_FILE "${CORE_MCU_DIR}/startup/startup_stm32f207xx.s")
set(SYSTEM_FILE "${CORE_MCU_DIR}/system/system_stm32f2xx.c")
