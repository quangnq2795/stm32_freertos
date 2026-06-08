#pragma once

typedef void (*cli_cmd_fn_t)(int argc, char **argv);

#define CLI_OK           0
#define CLI_ERR_PARAM   -1
#define CLI_ERR_BUSY    -2
#define CLI_ERR_FULL    -3

int cli_init(void);
void cli_uninit(void);
void cli_process(void);
int cli_register_command(const char *name, cli_cmd_fn_t handler);
