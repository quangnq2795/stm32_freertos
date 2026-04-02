#include "cli_cmd_led.h"

static void cmd_led(int argc, char **argv)
{
  (void)argc;
  (void)argv;
}

const cli_cmd_entry_t cli_led_cmds[] = {
    {"led", cmd_led},
    {0, 0},
};
