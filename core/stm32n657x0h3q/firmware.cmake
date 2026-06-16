set(CORE_MCU_DIR "${CMAKE_SOURCE_DIR}/core/${MCU_NAME}")
set(LINKER_SCRIPT "${CORE_MCU_DIR}/linker/STM32N657X0HXQ_AXISRAM2_fsbl.ld")
set(STARTUP_FILE "${CORE_MCU_DIR}/startup/startup_stm32n657x0hxq_fsbl.s")
set(SYSTEM_FILE "${CORE_MCU_DIR}/system/system_stm32n6xx_fsbl.c")
