#pragma once

#include "stm32f2xx_hal.h"

/*===========================================================================*/
/* Chọn nguồn đưa vào PLL → SYSCLK 120 MHz                                   */
/*===========================================================================*/

/*
 * Chỉ bật một đường:
 *
 *   BSP_CLOCK_PLL_SRC_HSE — cần HSE (thạch anh hoặc MCO 8 MHz trên NUCLEO).
 *   BSP_CLOCK_PLL_SRC_HSI — chỉ dùng HSI nội bộ (~16 MHz), không cần thạch anh.
 *
 * HSI: khởi động nhanh, không phụ thuộc mạch ngoài; độ chính xác / ổn định kém
 *      hơn HSE → UART/USB/RTC chuẩn tần số thường vẫn nên dùng HSE khi có.
 */
#define BSP_CLOCK_PLL_SRC_HSE 0
#define BSP_CLOCK_PLL_SRC_HSI 1

/* Mặc định HSE. Đổi sang HSI: sửa thành BSP_CLOCK_PLL_SRC_HSI hoặc build với
 * -DBSP_CLOCK_PLL_SRC=1 (1 = HSI, 0 = HSE). */
#define BSP_CLOCK_PLL_SRC BSP_CLOCK_PLL_SRC_HSE


/*===========================================================================*/
/* HSE — chỉ dùng khi BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE             */
/*===========================================================================*/

/*
 * Phải khớp HSE_VALUE trong configs/hal/.../stm32f2xx_hal_conf.h (NUCLEO: 8 MHz).
 *
 * RCC_HSE_BYPASS: OSC_IN nhận clock sẵn (MCO ST-Link).
 * RCC_HSE_ON: thạch anh giữa OSC_IN / OSC_OUT.
 */
#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE
#define BSP_CLOCK_HSE_STATE RCC_HSE_BYPASS
#endif

/*===========================================================================*/
/* PLL — STM32F207 (HAL: pllvco = f_src × PLLN / PLLM, SYSCLK = pllvco / P)   */
/*===========================================================================*/

/*
 * Với PLL làm SYSCLK và RCC_PLLP_DIV2 (chia 2):
 *   f_VCO   = f_PLL_src × PLLN / PLLM
 *   f_SYSCLK = f_VCO / 2
 *
 * f_PLL_src:
 *   - HSE: bằng HSE_VALUE (ví dụ 8 MHz) khi PLLSRC = HSE.
 *   - HSI: bằng HSI_VALUE (danh định 16 MHz trong hal_conf) khi PLLSRC = HSI.
 *
 * Điều kiện đầu vào PLL trên F2: f_PLL_src / PLLM nằm khoảng 1–2 MHz (xem RM).
 *
 * Cấu hình 120 MHz SYSCLK, VCO = 240 MHz, USB 48 MHz (f_VCO / PLLQ):
 *
 *   HSE 8 MHz,  M = 8  → 8/8   = 1 MHz → ×240 / 2 = 120 MHz, Q = 5 → 48 MHz.
 *   HSI 16 MHz, M = 16 → 16/16 = 1 MHz → ×240 / 2 = 120 MHz, Q = 5 → 48 MHz.
 *
 * Nếu đổi BSP_CLOCK_PLL_SRC sang HSI, phải dùng bộ PLLM/N/P/Q tương ứng cột HSI.
 * BSP_CLOCK_PLLM_* dùng trong SystemClock_Config_HSE/HSI; BSP_CLOCK_PLLM theo lựa chọn
 * hiện tại (tiện tham chiếu nơi khác).
 */
#define BSP_CLOCK_PLLN   240U
#define BSP_CLOCK_PLLP   RCC_PLLP_DIV2
#define BSP_CLOCK_PLLQ   5U
#define BSP_CLOCK_PLLM_HSE 8U
#define BSP_CLOCK_PLLM_HSI 16U

#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSI
#define BSP_CLOCK_PLLM BSP_CLOCK_PLLM_HSI
#else
#define BSP_CLOCK_PLLM BSP_CLOCK_PLLM_HSE
#endif

/*===========================================================================*/
/* Flash + bus — giữ nguyên khi vẫn 120 MHz HCLK / 30 MHz APB1 / 60 MHz APB2 */
/*===========================================================================*/

#define BSP_CLOCK_FLASH_LATENCY FLASH_LATENCY_3
#define BSP_CLOCK_AHB_DIV     RCC_SYSCLK_DIV1
#define BSP_CLOCK_APB1_DIV    RCC_HCLK_DIV4
#define BSP_CLOCK_APB2_DIV    RCC_HCLK_DIV2

/*
 * Sau khi đổi SYSCLK: sửa configCPU_CLOCK_HZ trong
 * configs/freertos/stm32f207zg/FreeRTOSConfig.h cho khớp (ở đây 120000000UL).
 */

