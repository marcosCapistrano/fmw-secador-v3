#include "state_manager.h"

#include "common.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "stdbool.h"
#include "storage.h"

extern QueueHandle_t state_msg_q;
extern QueueHandle_t ihm_msg_q;

typedef enum {
    STARTING,
    RUNNING,
    FINISHING,
} State_t;

static State_t curr_state = STARTING;

static const char *TAG = "STATE_MANAGER";

static void handle_starting(StateMessage_t *state_msg) {
    if (state_msg->type == STA_MSG_CONFIRM_NEW) {
        storage_start_new_lote();

        storage_add_record_init();
        storage_add_record_device_on();

        common_send_ihm_msg(IHM_MSG_RUN, NULL, portMAX_DELAY);
        common_send_perif_msg(PERIF_MSG_RUN, NULL, portMAX_DELAY);
        curr_state = RUNNING;
    } else if (state_msg->type == STA_MSG_CONFIRM_CONTINUE) {
        storage_add_record_device_on();

        common_send_ihm_msg(IHM_MSG_RUN, NULL, portMAX_DELAY);
        common_send_perif_msg(PERIF_MSG_RUN, NULL, portMAX_DELAY);
        curr_state = RUNNING;
    }
}

static void handle_running(StateMessage_t *state_msg) {
    if(state_msg->type == STA_MSG_FINISH) {
        storage_set_lote_concluded(true);
        common_send_ihm_msg(IHM_MSG_FINISH, (void *)NULL, portMAX_DELAY);
        ESP_LOGE(TAG, "Sending IHM msg");

        curr_state = STARTING;
    } else if (state_msg->type == STA_MSG_CHANGE_SENSOR_ENTR) {
        storage_set_sensor_entr(state_msg->payload);
        common_send_ihm_msg(IHM_MSG_CHANGE_SENSOR_ENTR, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_CONFIRM_CONTINUE) {
    }
}

static void handle_finishing(StateMessage_t *state_msg) {
    if (state_msg->type == STA_MSG_CONFIRM_NEW) {
        // Close Old Lote File
        // Create New Lote File
        // Add INIT State
        // Add ON State
    } else if (state_msg->type == STA_MSG_CONFIRM_CONTINUE) {
    }
}

static void state_manager_task(void *pvParameters) {
    StateMessage_t state_msg;

    for (;;) {
        if (xQueueReceive(state_msg_q, &state_msg, portMAX_DELAY)) {
            switch (curr_state) {
                case STARTING:
                    handle_starting(&state_msg);
                    break;

                case RUNNING:
                    handle_running(&state_msg);
                    break;

                case FINISHING:
                    handle_finishing(&state_msg);
                    break;

                default:
                    ESP_LOGE(TAG, "Unhandled State! %d", curr_state);
                    break;
            }
        }
    }
}

// switch (state_msg.type) {
//     case STA_MSG_CHANGE_QUEIMADOR_MODE:
//         storage_set_queimador_mode(state_msg.payload);
//         common_send_ihm_msg(IHM_MSG_CHANGE_QUEIMADOR_MODE, (void *)state_msg.payload, portMAX_DELAY);
//         break;

//     case STA_MSG_CHANGE_LIMIT_ENTR_MIN:
//         storage_set_min_entr(state_msg.payload);
//         break;

//     case STA_MSG_CHANGE_LIMIT_ENTR_MAX:
//         storage_set_max_entr(state_msg.payload);
//         common_send_ihm_msg(IHM_MSG_CHANGE_ENTR_LIMITS, (void *)NULL, portMAX_DELAY);
//         break;

//     case STA_MSG_CHANGE_LIMIT_M1_MIN:
//         storage_set_min_m1(state_msg.payload);
//         break;

//     case STA_MSG_CHANGE_LIMIT_M1_MAX:
//         storage_set_max_m1(state_msg.payload);
//         common_send_ihm_msg(IHM_MSG_CHANGE_M1_LIMITS, (void *)NULL, portMAX_DELAY);
//         break;

//     case STA_MSG_CHANGE_LIMIT_M2_MIN:
//         storage_set_min_m2(state_msg.payload);
//         break;

//     case STA_MSG_CHANGE_LIMIT_M2_MAX:
//         storage_set_max_m2(state_msg.payload);
//         common_send_ihm_msg(IHM_MSG_CHANGE_M2_LIMITS, (void *)NULL, portMAX_DELAY);
//         break;

//         storage_set_min_m3(state_msg.payload);
//         break;

//     case STA_MSG_CHANGE_LIMIT_M3_MAX:
//         storage_set_max_m3(state_msg.payload);
//         common_send_ihm_msg(IHM_MSG_CHANGE_M3_LIMITS, (void *)NULL, portMAX_DELAY);
//         break;

//     case STA_MSG_CHANGE_LIMIT_M4_MIN:
//         storage_set_min_m4(state_msg.payload);
//         break;

//     case STA_MSG_CHANGE_LIMIT_M4_MAX:
//         storage_set_max_m4(state_msg.payload);
//         common_send_ihm_msg(IHM_MSG_CHANGE_M4_LIMITS, (void *)NULL, portMAX_DELAY);
//         break;

//     case FINISHED:
//         ESP_LOGI(TAG, "Received event on FINISHED");
//         break;

//     case STA_MSG_CONFIRM_CONTINUE:
//         ESP_LOGE(TAG, "Received Confirm Continue!");
//         curr_state = RUNNING;
//         break;

//     case STA_MSG_CONFIRM_NEW:
//         break;

//     default:
//         ESP_LOGE(TAG, "Received Uhnandled Event: %d", state_msg.type);
//         break;
// }

void state_manager_init(void) {
    // Checar se estamos em new dry ou continue dry

    if (storage_get_lote_concluded()) {
        common_send_ihm_msg(IHM_MSG_NOTIFY_NEW_DRY, (void *)NULL, portMAX_DELAY);
    } else {
        common_send_ihm_msg(IHM_MSG_NOTIFY_CONTINUE_DRY, (void *)NULL, portMAX_DELAY);
    }

    xTaskCreate(state_manager_task, "STATE_MANAGER_TASK", 2400, NULL, 5, NULL);
}