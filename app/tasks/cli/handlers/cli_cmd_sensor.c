#include "cli_cmd_sensor.h"

static void cmd_sensor(int argc, char **argv)
{
  (void)argc;
  (void)argv;
}

const cli_cmd_entry_t cli_sensor_cmds[] = {
    {"sensor", cmd_sensor},
    {0, 0},
};
