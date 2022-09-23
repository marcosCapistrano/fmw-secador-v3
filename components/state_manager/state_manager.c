#include "state_manager.h"

#include "common.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "state_logic.h"
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
    if (state_msg->type == STA_MSG_FINISH) {
        storage_set_lote_concluded(true);
        common_send_ihm_msg(IHM_MSG_FINISH, (void *)NULL, portMAX_DELAY);

        curr_state = STARTING;
    } else if (state_msg->type == STA_MSG_CHANGE_SENSOR_ENTR) {
        storage_set_sensor_entr(sensor_entr);
        common_send_ihm_msg(IHM_MSG_CHANGE_SENSOR_ENTR, sensor_entr, portMAX_DELAY);

        PerifMessage_t alarme_action_msg = get_perif_alarme_action();
        PerifMessage_t queimador_action_msg = get_perif_queimador_action();
        PerifMessage_t led_entr_action_msg = get_perif_led_entr_action();
        IHMMessage_t ihm_action_msg = get_ihm_action();

        if (ihm_action_msg)
            common_send_ihm_msg(ihm_action_msg, (void *)NULL, portMAX_DELAY);

        if (alarme_action_msg)
            common_send_perif_msg(alarme_action_msg, (void *)NULL, portMAX_DELAY);

        if(queimador_action_msg)
            common_send_perif_msg(queimador_action_msg, (void *)NULL, portMAX_DELAY);

        if(led_entr_action_msg)
            common_send_perif_msg(led_entr_action_msg, (void *)NULL, portMAX_DELAY);

    } else if (state_msg->type == STA_MSG_CHANGE_SENSOR_M1) {
        int sensor_m1 = state_msg->payload;
        int limit_min = storage_get_min_m1();
        int limit_max = storage_get_max_m1();

        storage_set_sensor_m1(sensor_m1);
        common_send_ihm_msg(IHM_MSG_CHANGE_SENSOR_M1, sensor_m1, portMAX_DELAY);

        bool is_aware_m1 = storage_get_is_aware_m1();
        if (sensor_m1 < limit_min - HISTERESE && !is_aware_m1) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_LOW_M1, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_LOW_MASS, (void *)1, portMAX_DELAY);

            storage_set_should_alarme_m1(true);
            storage_set_should_queimador_m1(true);
        } else if (sensor_m1 > limit_max + HISTERESE && !is_aware_m1) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_HIGH_M1, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_HIGH_MASS, (void *)1, portMAX_DELAY);

            storage_set_should_alarme_m1(false);
            storage_set_should_queimador_m1(true);
        } else {
            storage_set_is_aware_m1(false);
            common_send_perif_msg(PERIF_MSG_NOTIFY_NORMAL_MASS, (void *)1, portMAX_DELAY);

            storage_set_should_alarme_m1(true);
            storage_set_should_queimador_m1(false);
        }
    } else if (state_msg->type == STA_MSG_CHANGE_SENSOR_M2) {
        int sensor_m2 = state_msg->payload;
        int limit_min = storage_get_min_m2();
        int limit_max = storage_get_max_m2();

        storage_set_sensor_m2(sensor_m2);
        common_send_ihm_msg(IHM_MSG_CHANGE_SENSOR_M2, sensor_m2, portMAX_DELAY);

        bool is_aware_m2 = storage_get_is_aware_m2();
        if (sensor_m2 < limit_min - HISTERESE && !is_aware_m2) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_LOW_M2, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_LOW_MASS, (void *)2, portMAX_DELAY);

            storage_set_should_alarme_m2(true);
            storage_set_should_queimador_m2(true);
        } else if (sensor_m2 > limit_max + HISTERESE && !is_aware_m2) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_HIGH_M1, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_HIGH_MASS, (void *)2, portMAX_DELAY);

            storage_set_should_alarme_m2(false);
            storage_set_should_queimador_m2(true);
        } else {
            storage_set_is_aware_m2(false);
            common_send_perif_msg(PERIF_MSG_NOTIFY_NORMAL_MASS, (void *)2, portMAX_DELAY);

            storage_set_should_alarme_m3(true);
            storage_set_should_queimador_m3(false);
        }
    } else if (state_msg->type == STA_MSG_CHANGE_SENSOR_M3) {
        int sensor_m3 = state_msg->payload;
        int limit_min = storage_get_min_m3();
        int limit_max = storage_get_max_m3();

        storage_set_sensor_m3(sensor_m3);
        common_send_ihm_msg(IHM_MSG_CHANGE_SENSOR_M3, sensor_m3, portMAX_DELAY);

        bool is_aware_m3 = storage_get_is_aware_m3();
        if (sensor_m3 < limit_min - HISTERESE && !is_aware_m3) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_LOW_M3, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_LOW_MASS, (void *)3, portMAX_DELAY);

            storage_set_should_alarme_m3(true);
            storage_set_should_queimador_m3(true);
        } else if (sensor_m3 > limit_max + HISTERESE && !is_aware_m3) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_HIGH_M3, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_HIGH_MASS, (void *)3, portMAX_DELAY);

            storage_set_should_alarme_m3(false);
            storage_set_should_queimador_m3(true);
        } else {
            storage_set_is_aware_m3(false);
            common_send_perif_msg(PERIF_MSG_NOTIFY_NORMAL_MASS, (void *)3, portMAX_DELAY);

            storage_set_should_alarme_m3(true);
            storage_set_should_queimador_m3(false);
        }
    } else if (state_msg->type == STA_MSG_CHANGE_SENSOR_M4) {
        int sensor_m4 = state_msg->payload;
        int limit_min = storage_get_min_m4();
        int limit_max = storage_get_max_m4();

        storage_set_sensor_m4(sensor_m4);
        common_send_ihm_msg(IHM_MSG_CHANGE_SENSOR_M4, sensor_m4, portMAX_DELAY);

        bool is_aware_m4 = storage_get_is_aware_m4();
        if (sensor_m4 < limit_min - HISTERESE && !is_aware_m4) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_LOW_M4, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_LOW_MASS, (void *)4, portMAX_DELAY);

            storage_set_should_alarme_m4(true);
            storage_set_should_queimador_m4(true);
        } else if (sensor_m4 > limit_max + HISTERESE && !is_aware_m4) {
            common_send_ihm_msg(IHM_MSG_NOTIFY_HIGH_M4, (void *)NULL, portMAX_DELAY);
            common_send_perif_msg(PERIF_MSG_NOTIFY_HIGH_MASS, (void *)4, portMAX_DELAY);

            storage_set_should_alarme_m4(false);
            storage_set_should_queimador_m4(true);
        } else {
            storage_set_is_aware_m4(false);
            common_send_perif_msg(PERIF_MSG_NOTIFY_NORMAL_MASS, (void *)4, portMAX_DELAY);

            storage_set_should_alarme_m4(true);
            storage_set_should_queimador_m4(false);
        }
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_ENTR_MIN) {
        storage_set_min_entr(state_msg->payload);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_ENTR_MAX) {
        storage_set_max_entr(state_msg->payload);
        common_send_ihm_msg(IHM_MSG_CONFIRM_LIMIT_ENTR, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M1_MIN) {
        storage_set_min_m1(state_msg->payload);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M1_MAX) {
        storage_set_max_m1(state_msg->payload);
        common_send_ihm_msg(IHM_MSG_CONFIRM_LIMIT_M1, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M2_MIN) {
        storage_set_min_m2(state_msg->payload);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M2_MAX) {
        storage_set_max_m2(state_msg->payload);
        common_send_ihm_msg(IHM_MSG_CONFIRM_LIMIT_M2, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M3_MIN) {
        storage_set_min_m3(state_msg->payload);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M3_MAX) {
        storage_set_max_m3(state_msg->payload);
        common_send_ihm_msg(IHM_MSG_CONFIRM_LIMIT_M3, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M4_MIN) {
        storage_set_min_m4(state_msg->payload);
    } else if (state_msg->type == STA_MSG_CHANGE_LIMIT_M4_MAX) {
        storage_set_max_m4(state_msg->payload);
        common_send_ihm_msg(IHM_MSG_CONFIRM_LIMIT_M4, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_CHANGE_QUEIMADOR_MODE) {
        storage_set_queimador_mode(state_msg->payload);
        common_send_ihm_msg(IHM_MSG_CHANGE_QUEIMADOR_MODE, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_CHANGE_CONNECT) {
        storage_set_connection(state_msg->payload, true);
        common_send_ihm_msg(IHM_MSG_CHANGE_CONNECT, state_msg->payload, portMAX_DELAY);
        common_send_perif_msg(PERIF_MSG_NOTIFY_CONNECTED, state_msg->payload, portMAX_DELAY);

        if (state_msg->payload == 1) {
            storage_set_should_connected_m1(true);
        } else if (state_msg->payload == 2) {
            storage_set_should_connected_m2(true);
        } else if (state_msg->payload == 3) {
            storage_set_should_connected_m3(true);
        } else if (state_msg->payload == 4) {
            storage_set_should_connected_m4(true);
        }
    } else if (state_msg->type == STA_MSG_CHANGE_DISCONNECT) {
        storage_set_connection(state_msg->payload, false);
        common_send_ihm_msg(IHM_MSG_CHANGE_DISCONNECT, state_msg->payload, portMAX_DELAY);
        common_send_perif_msg(PERIF_MSG_NOTIFY_DISCONNECTED, state_msg->payload, portMAX_DELAY);

        if (state_msg->payload == 1) {
            storage_set_should_connected_m1(false);
        } else if (state_msg->payload == 2) {
            storage_set_should_connected_m2(false);
        } else if (state_msg->payload == 3) {
            storage_set_should_connected_m3(false);
        } else if (state_msg->payload == 4) {
            storage_set_should_connected_m4(false);
        }
    } else if (state_msg->type == STA_MSG_NOTIFY_IS_AWARE_ENTR) {
        storage_set_is_aware_entr(true);
        common_send_ihm_msg(IHM_MSG_CONFIRM_IS_AWARE_ENTR, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_NOTIFY_IS_AWARE_M1) {
        storage_set_is_aware_m1(true);
        common_send_ihm_msg(IHM_MSG_CONFIRM_IS_AWARE_M1, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_NOTIFY_IS_AWARE_M2) {
        storage_set_is_aware_m2(true);
        common_send_ihm_msg(IHM_MSG_CONFIRM_IS_AWARE_M2, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_NOTIFY_IS_AWARE_M3) {
        storage_set_is_aware_m3(true);
        common_send_ihm_msg(IHM_MSG_CONFIRM_IS_AWARE_M3, state_msg->payload, portMAX_DELAY);
    } else if (state_msg->type == STA_MSG_NOTIFY_IS_AWARE_M4) {
        storage_set_is_aware_m4(true);
        common_send_ihm_msg(IHM_MSG_CONFIRM_IS_AWARE_M4, state_msg->payload, portMAX_DELAY);
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

    xTaskCreate(state_manager_task, "STATE_MANAGER_TASK", 4800, NULL, 5, NULL);
}