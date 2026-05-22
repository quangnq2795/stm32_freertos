#include "driver.h"

void os_init(void);
void os_start(void);

int main(void)
{
  driver_init();
  os_init();
  os_start();
  while (1) {}
}
