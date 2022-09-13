#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "state_manager.h"
#include "server_controller.h"
#include "ihm_controller.h"
#include "perif_controller.h"
#include "storage.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "common.h"

static const char *TAG = "MAIN";

QueueHandle_t state_msg_q;
QueueHandle_t ihm_msg_q;
QueueHandle_t perif_msg_q;
QueueHandle_t server_msg_q;

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS Ok!");

    ESP_LOGI(TAG, "Iniciando todas as Queues");
    state_msg_q = xQueueCreate(10, sizeof(StateMessage_t));
    ihm_msg_q = xQueueCreate(IHM_UPDATE_QUEUE_LEN, sizeof(IHMMessage_t));
    perif_msg_q = xQueueCreate(10, sizeof(PerifMessage_t));
    server_msg_q = xQueueCreate(10, sizeof(ServerMessage_t));
    ESP_LOGI(TAG, "Queues OK!");

    ESP_LOGI(TAG, "Iniciando todos Módulos...");
    state_manager_init();
    storage_init();
    perif_controller_init();
    server_controller_init();
    ihm_controller_init();
    ESP_LOGI(TAG, "Modulos OK!");
}