#include "common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

extern QueueHandle_t state_msg_q;
extern QueueHandle_t ihm_msg_q;
extern QueueHandle_t perif_msg_q;
extern QueueHandle_t server_msg_q;

void common_send_state_msg(StateMessageType_t type, void *payload, portTickType ticks_to_wait) {
    StateMessage_t state_msg = {
        .type = type,
        .payload = payload,
    };

    xQueueSend(state_msg_q, (void *)&state_msg, ticks_to_wait);
}

void common_send_ihm_msg(IHMMessageType_t type, void *payload, portTickType ticks_to_wait) {
    IHMMessage_t ihm_msg = {
        .type = type,
        .payload = payload,
    };

    xQueueSend(ihm_msg_q, (void *)&ihm_msg, ticks_to_wait);
}

void common_send_perif_msg(PerifMessageType_t type, void *payload, portTickType ticks_to_wait) {
    PerifMessage_t perif_msg = {
        .type = type,
        .payload = payload,
    };

    xQueueSend(perif_msg_q, (void *)&perif_msg, ticks_to_wait);
}