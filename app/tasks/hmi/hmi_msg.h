#pragma once

#include <stdint.h>

/* Fixed-size queue item: CLI (producer) -> HMI (consumer). */

#ifndef HMI_LCD_TEXT_MAX
#define HMI_LCD_TEXT_MAX 48U
#endif

typedef enum {
  HMI_MSG_LED_SET = 0,
  HMI_MSG_LED_BLINK,
  HMI_MSG_LCD_TEXT,
} hmi_msg_type_t;

typedef struct
{
  uint8_t led_id; /* 0 .. BSP_LED_COUNT-1 */
  uint8_t on;   /* 0 = off, non-zero = on (tắt blink LED này). */
} hmi_msg_led_set_t;

typedef struct
{
  uint8_t led_id; /* 0 .. BSP_LED_COUNT-1 */
  uint16_t period_ms; /* Chu kỳ đầy đủ (on+off); 0 = tắt blink, LED off. */
} hmi_msg_led_blink_t;

typedef struct
{
  uint8_t row;
  uint8_t col;
  char text[HMI_LCD_TEXT_MAX]; /* Chuỗi kết thúc '\0', cắt bớt nếu quá dài khi copy vào đây. */
} hmi_msg_lcd_text_t;

typedef struct
{
  hmi_msg_type_t type;
  union
  {
    hmi_msg_led_set_t led_set;
    hmi_msg_led_blink_t led_blink;
    hmi_msg_lcd_text_t lcd_text;
  } u;
} hmi_msg_t;
