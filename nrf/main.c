/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : main.c
Purpose : Generic application start

*/

#include "app_error.h"
#include "app_util_platform.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include <stdio.h>
#include <string.h>

/*********************************************************************
 *
 *       main()
 *
 *  Function description
 *   Application entry point.
 */
int main(void) {
  int i;

  for (i = 0; i < 100; i++) {
    printf("Hello World %d!\n", i);
  }
  do {
    i++;
  } while (1);
}

/*************************** End of file ****************************/
