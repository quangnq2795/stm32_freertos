#include "driver.h"

#include "stm32_hal.h"
#include "bsp_smps_cfg.h"
#include "clk.h"
#include "gpio.h"

/* Mark the linker .noncacheable section as Not-Cacheable via MPU region 0.
 * Required before enabling the Cortex-M55 D-cache so DMA/peripheral-shared
 * buffers placed there stay coherent. */
static void mpu_config_noncacheable(void)
{
  MPU_Region_InitTypeDef region = {0};
  MPU_Attributes_InitTypeDef attr = {0};
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  HAL_MPU_Disable();

  attr.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);
  attr.Number = MPU_ATTRIBUTES_NUMBER0;
  HAL_MPU_ConfigMemoryAttributes(&attr);

  region.Enable = MPU_REGION_ENABLE;
  region.Number = MPU_REGION_NUMBER0;
  region.BaseAddress = __NON_CACHEABLE_SECTION_BEGIN;
  region.LimitAddress = __NON_CACHEABLE_SECTION_END;
  region.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  region.AccessPermission = MPU_REGION_ALL_RW;
  region.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  region.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
  HAL_MPU_ConfigRegion(&region);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  __set_PRIMASK(primask);
}

/* Drive the board's external SMPS select pin HIGH = overdrive voltage, needed
 * before raising the core to 800 MHz. Replaces the STM32Cube BSP_SMPS_Init(). */
static void smps_init_overdrive(void)
{
  gpio_config_output(BSP_SMPS_GPIO_PORT, BSP_SMPS_GPIO_PIN, GPIO_OUT_PUSH_PULL,
                     GPIO_NOPULL, GPIO_LEVEL_HIGH);
}

void driver_init(void)
{
  /* Pre-HAL: MPU non-cacheable region, then enable caches (Cortex-M55). */
  mpu_config_noncacheable();
  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_Init();

  /* Post-HAL: set board SMPS to overdrive before clk_init raises SYSCLK. */
  smps_init_overdrive();

  clk_init();
}
