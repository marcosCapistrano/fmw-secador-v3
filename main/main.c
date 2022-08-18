#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "ihm_manager.h"
#include "controller.h"

// #define PRODUCTION

QueueHandle_t ihm_update_q;
QueueHandle_t controller_update_q;

void app_main(void)
{
    ihm_manager_init();
    controller_init();
}