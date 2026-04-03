#include "hmi_lcd.h"

#include <stddef.h>

static void hmi_lcd_put_text(uint8_t row, uint8_t col, const char *text)
{
  (void)row;
  (void)col;
  (void)text;
  /* Stub: gắn driver LCD (I2C/SPI/parallel) tại đây. */
}

void hmi_lcd_init(void)
{
}

void hmi_lcd_on_msg(const hmi_msg_t *msg)
{
  if (msg == NULL || msg->type != HMI_MSG_LCD_TEXT) {
    return;
  }
  hmi_lcd_put_text(msg->u.lcd_text.row, msg->u.lcd_text.col, msg->u.lcd_text.text);
}
