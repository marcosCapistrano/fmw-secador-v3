#include "state_manager.h"

#include "stdbool.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "storage.h"
#include "common.h"

extern QueueHandle_t state_msg_q;
extern QueueHandle_t ihm_msg_q;

static const char *TAG = "STATE_MANAGER";

static void state_manager_task(void *pvParameters) {
    StateMessage_t state_msg;

    for (;;) {
        if (xQueueReceive(state_msg_q, &state_msg, portMAX_DELAY)) {
            switch(state_msg.type) {
                case STA_MSG_CHANGE_QUEIMADOR_MODE:
                    storage_set_queimador_mode(state_msg.payload);

                    ESP_LOGE(TAG, "Payload: %d", (int) state_msg.payload);

                    IHMMessage_t ihm_msg = {
                        .type = IHM_MSG_CHANGE_QUEIMADOR_MODE,
                        .payload = state_msg.payload,
                    };

                    xQueueSend(ihm_msg_q, (void*)&ihm_msg, portMAX_DELAY);
                break;
            }
        }
    }
}

void state_manager_init(void) {
    xTaskCreate(state_manager_task, "STATE_MANAGER_TASK", 2400, NULL, 5, NULL);
    // Salvar horário de ligamento
}