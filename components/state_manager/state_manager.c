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
                    common_send_ihm_msg(IHM_MSG_CHANGE_QUEIMADOR_MODE, (void *) state_msg.payload, portMAX_DELAY);
                break;

                case STA_MSG_CHANGE_LIMIT_ENTR_MIN:
                    storage_set_min_entr(state_msg.payload);
                break;

                case STA_MSG_CHANGE_LIMIT_ENTR_MAX:
                    storage_set_max_entr(state_msg.payload);
                    common_send_ihm_msg(IHM_MSG_CHANGE_ENTR_LIMITS, (void *)NULL, portMAX_DELAY);
                break;

                case STA_MSG_CHANGE_LIMIT_M1_MIN:
                    storage_set_min_m1(state_msg.payload);
                break;

                case STA_MSG_CHANGE_LIMIT_M1_MAX:
                    storage_set_max_m1(state_msg.payload);
                    common_send_ihm_msg(IHM_MSG_CHANGE_M1_LIMITS, (void *)NULL, portMAX_DELAY);
                break;

                case STA_MSG_CHANGE_LIMIT_M2_MIN:
                    storage_set_min_m2(state_msg.payload);
                break;

                case STA_MSG_CHANGE_LIMIT_M2_MAX:
                    storage_set_max_m2(state_msg.payload);
                    common_send_ihm_msg(IHM_MSG_CHANGE_M2_LIMITS, (void *)NULL, portMAX_DELAY);
                break;

                case STA_MSG_CHANGE_LIMIT_M3_MIN:
                    storage_set_min_m3(state_msg.payload);
                break;

                case STA_MSG_CHANGE_LIMIT_M3_MAX:
                    storage_set_max_m3(state_msg.payload);
                    common_send_ihm_msg(IHM_MSG_CHANGE_M3_LIMITS, (void *)NULL, portMAX_DELAY);
                break;

                case STA_MSG_CHANGE_LIMIT_M4_MIN:
                    storage_set_min_m4(state_msg.payload);
                break;

                case STA_MSG_CHANGE_LIMIT_M4_MAX:
                    storage_set_max_m4(state_msg.payload);
                    common_send_ihm_msg(IHM_MSG_CHANGE_M4_LIMITS, (void *)NULL, portMAX_DELAY);
                break;
            }
        }
    }
}

void state_manager_init(void) {
    xTaskCreate(state_manager_task, "STATE_MANAGER_TASK", 2400, NULL, 5, NULL);
    // Salvar horário de ligamento
}