set(CORE_MCU_DIR "${CMAKE_SOURCE_DIR}/core/mcu/${MCU_NAME}")
set(LINKER_SCRIPT "${CORE_MCU_DIR}/linker/STM32F207ZGTx_FLASH.ld")
set(STARTUP_FILE "${CORE_MCU_DIR}/startup/startup_stm32f207xx.s")
set(SYSTEM_FILE "${CORE_MCU_DIR}/system/system_stm32f2xx.c")
