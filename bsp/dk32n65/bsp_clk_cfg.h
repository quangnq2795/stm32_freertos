#pragma once

#include "stm32_hal.h"

/*===========================================================================*/
/* Chọn nguồn đưa vào PLL1 → CPU 800 MHz                                      */
/*===========================================================================*/

/*
 * Chỉ bật một đường:
 *
 *   BSP_CLOCK_PLL_SRC_HSI — HSI nội bộ 64 MHz, không cần mạch ngoài. Mặc định.
 *   BSP_CLOCK_PLL_SRC_HSE — cần HSE (thạch anh 48 MHz trên STM32N6570-DK).
 *
 * HSI: khởi động nhanh, không phụ thuộc linh kiện ngoài; độ chính xác / ổn định
 *      kém hơn HSE → các ngoại vi cần chuẩn tần số (UART tốc độ cao, USB...) nên
 *      dùng HSE khi có.
 */
#define BSP_CLOCK_PLL_SRC_HSE 0
#define BSP_CLOCK_PLL_SRC_HSI 1

/* Mặc định HSI. Đổi sang HSE: sửa giá trị dưới đây hoặc build với
 * -DBSP_CLOCK_PLL_SRC=0 (0 = HSE, 1 = HSI). Guard #ifndef để cờ -D đè được. */
#ifndef BSP_CLOCK_PLL_SRC
#define BSP_CLOCK_PLL_SRC BSP_CLOCK_PLL_SRC_HSI
#endif

/*===========================================================================*/
/* HSE — chỉ dùng khi BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE             */
/*===========================================================================*/

/*
 * Phải khớp HSE_VALUE trong config/hal/.../stm32n6xx_hal_conf.h (DK: 48 MHz).
 *
 * RCC_HSE_ON:             thạch anh nối giữa OSC_IN / OSC_OUT (mặc định DK).
 * RCC_HSE_BYPASS:         OSC_IN nhận clock sin/analog từ nguồn ngoài.
 * RCC_HSE_BYPASS_DIGITAL: OSC_IN nhận clock số (square) từ nguồn ngoài.
 */
#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE
#define BSP_CLOCK_HSE_STATE RCC_HSE_ON
#endif

/*===========================================================================*/
/* Đích tần số CPU                                                            */
/*===========================================================================*/

/* CPUCLK mong muốn = đầu ra PLL1 / IC1_DIV (xem bên dưới). Tham chiếu cho nơi
 * khác; phải khớp configCPU_CLOCK_HZ nếu FreeRTOS dùng tới. */
#define BSP_CLOCK_CPU_HZ      800000000UL

/*===========================================================================*/
/* PLL1 — STM32N6 (ref = f_src / PLLM; VCO = ref × PLLN; out = VCO / P1 / P2)  */
/*===========================================================================*/

/*
 * Chuỗi tạo CPUCLK:
 *   f_ref  = f_src / PLLM                 (đầu vào bộ so pha PLL)
 *   f_VCO  = f_ref × PLLN
 *   f_PLL1 = f_VCO / (PLLP1 × PLLP2)      (đầu ra PLL1)
 *   CPUCLK = f_PLL1 / IC1_DIV
 *
 * f_src:
 *   - HSI: HSI_VALUE = 64 MHz (sau HSIDiv = /1).
 *   - HSE: HSE_VALUE = 48 MHz.
 *
 * Giữ ref = 8 MHz cho cả hai nguồn để dùng chung PLLN/P1/P2:
 *   HSI 64 MHz, M = 8 → 64/8 = 8 MHz → ×100 = 800 MHz VCO → /1 /1 = 800 MHz.
 *   HSE 48 MHz, M = 6 → 48/6 = 8 MHz → ×100 = 800 MHz VCO → /1 /1 = 800 MHz.
 *
 * Đổi BSP_CLOCK_PLL_SRC chỉ cần đổi PLLM tương ứng (M tự chọn theo nguồn dưới đây).
 */
#define BSP_CLOCK_PLLM_HSI 8U
#define BSP_CLOCK_PLLM_HSE 6U

#if BSP_CLOCK_PLL_SRC == BSP_CLOCK_PLL_SRC_HSE
#define BSP_CLOCK_PLLM BSP_CLOCK_PLLM_HSE
#else
#define BSP_CLOCK_PLLM BSP_CLOCK_PLLM_HSI
#endif

#define BSP_CLOCK_PLLN   100U  /* nhân VCO: 8 MHz × 100 = 800 MHz              */
#define BSP_CLOCK_PLLP1  1U    /* hậu chia tầng 1 (không chia)                */
#define BSP_CLOCK_PLLP2  1U    /* hậu chia tầng 2 (không chia) → PLL1 800 MHz  */

/*===========================================================================*/
/* IC (Internal Clock) — chia PLL1 ra các domain CPU/SYS                      */
/*===========================================================================*/

/*
 * Mỗi bộ IC lấy PLL1 (800 MHz) rồi chia tiếp:
 *   IC1  → CPUCLK          : /1 = 800 MHz (lõi Cortex-M55)
 *   IC2  → SYSCLK (bus hệ) : /2 = 400 MHz
 *   IC6  → SYSCLK          : /4 = 200 MHz
 *   IC11 → SYSCLK          : /2 = 400 MHz
 */
#define BSP_CLOCK_IC1_DIV 1U
#define BSP_CLOCK_IC2_DIV 2U
#define BSP_CLOCK_IC6_DIV 4U
#define BSP_CLOCK_IC11_DIV 2U

/*===========================================================================*/
/* Hệ số chia bus (tính từ SYSCLK)                                            */
/*===========================================================================*/

#define BSP_CLOCK_AHB_DIV     RCC_HCLK_DIV2   /* HCLK = SYSCLK / 2 */
#define BSP_CLOCK_APB1_DIV    RCC_APB1_DIV1
#define BSP_CLOCK_APB2_DIV    RCC_APB2_DIV1
#define BSP_CLOCK_APB4_DIV    RCC_APB4_DIV1
#define BSP_CLOCK_APB5_DIV    RCC_APB5_DIV1
