#include "bsp.h"

void os_init(void);
void os_start(void);

int main(void)
{
  bsp_init();
  os_init();
  os_start();
  while (1) {}
}
