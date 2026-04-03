#include "cli_cmd_lcd.h"

#include "hmi_msg.h"
#include "task.cli.h"
#include "task_hmi.h"

#include <stdlib.h>
#include <string.h>

static void cmd_lcd(int argc, char **argv)
{
  if (argc < 4) {
    cli_print("usage: lcd <row> <col> text...\r\n");
    return;
  }

  unsigned long const row = strtoul(argv[1], NULL, 0);
  unsigned long const col = strtoul(argv[2], NULL, 0);

  hmi_msg_t msg;
  memset(&msg, 0, sizeof(msg));
  msg.type = HMI_MSG_LCD_TEXT;
  msg.u.lcd_text.row = (uint8_t)row;
  msg.u.lcd_text.col = (uint8_t)col;

  char *dst = msg.u.lcd_text.text;
  size_t cap = HMI_LCD_TEXT_MAX;
  size_t pos = 0U;
  dst[0] = '\0';

  for (int i = 3; i < argc; ++i) {
    if (i > 3 && pos < cap) {
      dst[pos++] = ' ';
    }
    size_t const chunk = strlen(argv[i]);
    size_t n = chunk;
    if (pos + n >= cap) {
      n = (cap > pos + 1U) ? (cap - 1U - pos) : 0U;
    }
    if (n > 0U) {
      memcpy(dst + pos, argv[i], n);
      pos += n;
      dst[pos] = '\0';
    }
  }

  if (!hmi_cmd_send(&msg, pdMS_TO_TICKS(50))) {
    cli_print("hmi queue full\r\n");
  }
}

const cli_cmd_entry_t cli_lcd_cmds[] = {
    {"lcd", cmd_lcd},
    {0, 0},
};
