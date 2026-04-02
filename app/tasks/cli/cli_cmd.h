#pragma once

typedef void (*cli_cmd_fn_t)(int argc, char **argv);

typedef struct
{
  const char *name;
  cli_cmd_fn_t fn;
} cli_cmd_entry_t;
