#include "ihm_controller.h"

#include <stdbool.h>
#include <string.h>

#include "common.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_nextion.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "storage.h"

#define UART_NUM (UART_NUM_2)
#define TXD_PIN 17
#define RXD_PIN 16

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

typedef enum {
    STARTING,
    RUNNING,
} State_t;

typedef struct {
    uint8_t curr_page;
    State_t curr_state;
} IHMState_t;

static IHMState_t ihm_state = {
    .curr_page = 0,
    .curr_state = STARTING};

static QueueSetHandle_t ihm_qs;
static QueueHandle_t ihm_input_q;
extern QueueHandle_t ihm_msg_q;
extern QueueHandle_t state_msg_q;

static const char *TAG = "IHM_MANAGER";

typedef enum {
    LIMIT_NONE,
    ENTR,
    M1,
    M2,
    M3,
    M4
} LimitType_t;

typedef struct {
    LimitType_t type;
    int limit_min;
    int limit_max;
} LimitPageDetails_t;

static LimitPageDetails_t limit_page_details = {
    .type = LIMIT_NONE,
    .limit_min = -1,
    .limit_max = -1,
};

static void write_to_ihm(const char *command) {
    size_t length = strlen(command);

    ESP_LOGE(TAG, "Sending command: %s", command);

    uart_write_bytes(UART_NUM, "\xFF\xFF\xFF", 3);
    uart_write_bytes(UART_NUM, command, length);
    uart_write_bytes(UART_NUM, "\xFF\xFF\xFF", 3);
}

static void write_text_temperature(int tNumber, int value) {
    char temp_buf[25] = {0};

    nex_text_change_temp(temp_buf, tNumber, value);
    write_to_ihm(temp_buf);
}

static void write_number_temperature(int tNumber, int value) {
    char temp_buf[25] = {0};

    nex_number_change_temp(temp_buf, tNumber, value);
    write_to_ihm(temp_buf);
}

static void write_text_lote_number(int tNumber, uint8_t new_lote_number) {
    char temp_buf[25] = {0};

    nex_text_change_lote_number(temp_buf, tNumber, new_lote_number);
    write_to_ihm(temp_buf);
}

static void write_pic_id(int pNumber, int value) {
    char temp_buf[25] = {0};

    nex_pic_change_pic_id(temp_buf, pNumber, value);
    write_to_ihm(temp_buf);
}

static void write_button_pic_id(int bNumber, int value) {
    char temp_buf[25] = {0};

    nex_button_change_pic_id(temp_buf, bNumber, value);
    write_to_ihm(temp_buf);

    memset(temp_buf, 0, 25 * (sizeof temp_buf[0]));

    nex_button_change_pic2_id(temp_buf, bNumber, value);
    write_to_ihm(temp_buf);
}

static void write_change_page(int value) {
    char temp_buf[25] = {0};

    nex_change_page(temp_buf, value);
    write_to_ihm(temp_buf);
}

static void write_queimador_mode(bool mode) {
    if (mode) {
        write_button_pic_id(5, 36);
    } else {
        write_button_pic_id(5, 37);
    }
}

static int extract_number_from_get(uint8_t *buf, int start, int end) {
    return buf[start + 1];
}

static void dispatch_button_released(uint8_t page_id, uint8_t component_id) {
    ESP_LOGE(TAG, "button released! %d, compo: %d", page_id, component_id);
    if (page_id == 1) {           // Pagina sensores
        if (component_id == 6) {  // Entrada
            write_change_page(2);
        } else if (component_id == 7) {  // Massa 1
            write_change_page(3);
        } else if (component_id == 8) {  // Massa 2
            write_change_page(4);
        } else if (component_id == 9) {  // Massa 3
            write_change_page(5);
        } else if (component_id == 10) {  // Massa 4
            write_change_page(6);
        } else if (component_id == 15) {         // Palha Lenha
            if (storage_get_queimador_mode()) {  // Se tiver na lenha
                common_send_state_msg(STA_MSG_CHANGE_QUEIMADOR_MODE, (void *)false, portMAX_DELAY);
            } else {  // Se tiver na palha
                common_send_state_msg(STA_MSG_CHANGE_QUEIMADOR_MODE, (void *)true, portMAX_DELAY);
            }
        } else if (component_id == 16) {  // Finalizar Seca
            common_send_state_msg(STA_MSG_FINISH, (void *)NULL, portMAX_DELAY);
        }

    } else if (page_id == 2) {    // Pagina Limit Entr
        if (component_id == 5) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 6) {  // Aplicar
            limit_page_details.type = ENTR;
            write_to_ihm("get n0.val");
            write_to_ihm("get n1.val");
        }
    } else if (page_id == 3) {    // Pagina Limit M1
        if (component_id == 5) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 6) {  // Aplicar
            limit_page_details.type = M1;
            write_to_ihm("get n0.val");
            write_to_ihm("get n1.val");
        }
    } else if (page_id == 4) {    // Pagina Limit M2
        if (component_id == 5) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 6) {  // Aplicar
            limit_page_details.type = M2;
            write_to_ihm("get n0.val");
            write_to_ihm("get n1.val");
        }
    } else if (page_id == 5) {  // Pagina Limit M3

        if (component_id == 5) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 6) {  // Aplicar
            limit_page_details.type = M3;
            write_to_ihm("get n0.val");
            write_to_ihm("get n1.val");
        }
    } else if (page_id == 6) {    // Pagina Limit M4
        if (component_id == 5) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 6) {  // Aplicar
            limit_page_details.type = M4;
            write_to_ihm("get n0.val");
            write_to_ihm("get n1.val");
        }
    } else if (page_id == 7) {  // Pagina Low Entr
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_ENTR, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 8) {  // Pagina High Entr
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_ENTR, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 9) {  // Pagina Low M1
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M1, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 10) {  // Pagina High M1
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M1, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 11) {  // Pagina Low M2
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M2, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 12) {  // Pagina High M2
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M2, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 13) {  // Pagina Low M3
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M3, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 14) {  // Pagina High M3
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M3, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 15) {  // Pagina Low M4
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M4, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 16) {  // Pagina High M4
        common_send_state_msg(STA_MSG_NOTIFY_IS_AWARE_M4, (void *)NULL, portMAX_DELAY);
    } else if (page_id == 17) {
        common_send_state_msg(STA_MSG_CONFIRM_NEW, (void *)NULL, portMAX_DELAY);
    }
}

static void dispatch_page_loaded(uint8_t page_id) {
    ihm_state.curr_page = page_id;
    switch (page_id) {
        case 1: {  // Pagina Sensores
            bool m1_connected = storage_get_conexao_m1();
            uint8_t sensor_m1 = storage_get_sensor_m1();

            bool m2_connected = storage_get_conexao_m2();
            uint8_t sensor_m2 = storage_get_sensor_m2();

            bool m3_connected = storage_get_conexao_m3();
            uint8_t sensor_m3 = storage_get_sensor_m3();

            bool m4_connected = storage_get_conexao_m4();
            uint8_t sensor_m4 = storage_get_sensor_m4();

            uint8_t sensor_entr = storage_get_sensor_entr();
            bool queimador_mode = storage_get_queimador_mode();

            write_text_temperature(0, sensor_entr);

            if (m1_connected) {
                write_pic_id(1, 30);
                write_text_temperature(1, sensor_m1);
            } else {
                write_pic_id(1, 29);
                write_text_temperature(1, -1);
            }

            if (m2_connected) {
                write_pic_id(2, 30);
                write_text_temperature(2, sensor_m2);
            } else {
                write_pic_id(2, 29);
                write_text_temperature(1, -1);
            }

            if (m3_connected) {
                write_pic_id(3, 30);
                write_text_temperature(3, sensor_m3);
            } else {
                write_pic_id(3, 29);
                write_text_temperature(1, -1);
            }

            if (m4_connected) {
                write_pic_id(4, 30);
                write_text_temperature(4, sensor_m4);
            } else {
                write_pic_id(4, 29);
                write_text_temperature(1, -1);
            }

            write_queimador_mode(queimador_mode);
        } break;

        case 2: {  // Pagina Limit Entr
            uint8_t limit_min = storage_get_min_entr();
            uint8_t limit_max = storage_get_max_entr();

            write_number_temperature(0, limit_min);
            write_number_temperature(1, limit_max);
        } break;

        case 3: {  // Pagina Limit M1
            uint8_t limit_min = storage_get_min_m1();
            uint8_t limit_max = storage_get_max_m1();

            write_number_temperature(0, limit_min);
            write_number_temperature(1, limit_max);
        } break;

        case 4: {  // Pagina Limit M2
            uint8_t limit_min = storage_get_min_m2();
            uint8_t limit_max = storage_get_max_m2();

            write_number_temperature(0, limit_min);
            write_number_temperature(1, limit_max);
        } break;

        case 5: {  // Pagina Limit M3
            uint8_t limit_min = storage_get_min_m3();
            uint8_t limit_max = storage_get_max_m3();

            write_number_temperature(0, limit_min);
            write_number_temperature(1, limit_max);
        } break;

        case 6: {  // Pagina Limit M4
            uint8_t limit_min = storage_get_min_m4();
            uint8_t limit_max = storage_get_max_m4();

            write_number_temperature(0, limit_min);
            write_number_temperature(1, limit_max);
        } break;

        case 17: {
            uint8_t new_lote_number = storage_get_lote_number();
            new_lote_number++;

            write_text_lote_number(0, new_lote_number);
        }
    }
}

static void reset_temp_limits() {
    limit_page_details.type = LIMIT_NONE;
    limit_page_details.limit_min = -1;
    limit_page_details.limit_max = -1;
}

static void send_limits_changed() {
    switch (limit_page_details.type) {
        case ENTR: {
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_ENTR_MIN, (void *)limit_page_details.limit_min, portMAX_DELAY);
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_ENTR_MAX, (void *)limit_page_details.limit_max, portMAX_DELAY);
        } break;
        case M1: {
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M1_MIN, (void *)limit_page_details.limit_min, portMAX_DELAY);
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M1_MAX, (void *)limit_page_details.limit_max, portMAX_DELAY);
        } break;
        case M2: {
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M2_MIN, (void *)limit_page_details.limit_min, portMAX_DELAY);
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M2_MAX, (void *)limit_page_details.limit_max, portMAX_DELAY);
        } break;
        case M3: {
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M3_MIN, (void *)limit_page_details.limit_min, portMAX_DELAY);
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M3_MAX, (void *)limit_page_details.limit_max, portMAX_DELAY);
        } break;
        case M4: {
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M4_MIN, (void *)limit_page_details.limit_min, portMAX_DELAY);
            common_send_state_msg(STA_MSG_CHANGE_LIMIT_M4_MAX, (void *)limit_page_details.limit_max, portMAX_DELAY);
        } break;

        default:
            ESP_LOGE(TAG, "Error sending Limit changes to StateManager, no type set on LimitPageDetails");
    }
}

static void dispatch_get_number_response(uint8_t *buf, int start, int end) {
    if (ihm_state.curr_page == 2 || ihm_state.curr_page == 3 || ihm_state.curr_page == 4 || ihm_state.curr_page == 5 || ihm_state.curr_page == 6) {
        if (limit_page_details.limit_min == -1) {
            limit_page_details.limit_min = extract_number_from_get(buf, start, end);
        } else if (limit_page_details.limit_max == -1) {
            limit_page_details.limit_max = extract_number_from_get(buf, start, end);

            send_limits_changed();
            reset_temp_limits();
        }
    }
}

static void dispatch_continue_timer_expired() {
    if (ihm_state.curr_page == 18) {
        common_send_state_msg(STA_MSG_CONFIRM_CONTINUE, (void *)NULL, portMAX_DELAY);
    } else if (ihm_state.curr_page == 19) {
        ihm_state.curr_state = STARTING;
        write_change_page(17);
    }
}

static void process_command(uint8_t *data, int start, int end) {
    size_t length = end - start + 1;
    uint8_t data_head = data[start];

    if (data_head == 101) {
        uint8_t data_page_id = data[start + 1];
        uint8_t data_component_id = data[start + 2];
        dispatch_button_released(data_page_id, data_component_id);
    } else if (data_head == 102) {
        uint8_t data_page_id = data[start + 1];
        dispatch_page_loaded(data_page_id);
    } else if (data_head == 113) {
        dispatch_get_number_response(data, start, end);
    } else {
        dispatch_continue_timer_expired();
    }
}

static void process_input(uart_event_t *uart_event) {
    size_t buffer_size = uart_event->size;
    uint8_t data[128];

    /*
    Checa se o tipo de evento ?? do tipo 0 (DATA), se n??o for, analisar pq pode ser fonte de bugs
    */
    if (uart_event->type != 0) {
        ESP_LOGE(TAG, "Uart Event Type returned: %d", uart_event->type);
    }

    uart_read_bytes(UART_NUM, data, buffer_size, portMAX_DELAY);

    int command_start = 0;
    int command_end = 0;
    while (command_start < buffer_size) {
        for (int i = command_start; i < buffer_size; i++) {
            if (data[i] == 255) {
                command_end = i + 2;
                break;
            }
        }

        if (command_end == 0) {
            command_end = buffer_size - 1;
        }

        process_command(data, command_start, command_end);
        command_start = command_end + 1;
    }
}

static void handle_update_starting(IHMMessage_t *update_event) {
    if (update_event->type == IHM_MSG_NOTIFY_NEW_DRY) {
        write_change_page(17);
    } else if (update_event->type == IHM_MSG_NOTIFY_CONTINUE_DRY) {
        write_change_page(18);
    } else if (update_event->type == IHM_MSG_RUN) {
        write_change_page(1);
        ihm_state.curr_state = RUNNING;
    }
}

static void handle_update_running(IHMMessage_t *update_event) {
    if (update_event->type == IHM_MSG_FINISH) {
        write_change_page(19);
        ihm_state.curr_state = STARTING;
    } else if (update_event->type == IHM_MSG_CHANGE_SENSOR_ENTR) {
        if (ihm_state.curr_page == 1)
            write_text_temperature(0, update_event->payload);
    } else if (update_event->type == IHM_MSG_CHANGE_SENSOR_M1) {
        if (ihm_state.curr_page == 1)
            write_text_temperature(1, update_event->payload);
    } else if (update_event->type == IHM_MSG_CHANGE_SENSOR_M2) {
        if (ihm_state.curr_page == 1)
            write_text_temperature(2, update_event->payload);
    } else if (update_event->type == IHM_MSG_CHANGE_SENSOR_M3) {
        if (ihm_state.curr_page == 1)
            write_text_temperature(3, update_event->payload);
    } else if (update_event->type == IHM_MSG_CHANGE_SENSOR_M4) {
        if (ihm_state.curr_page == 1)
            write_text_temperature(4, update_event->payload);
    } else if (update_event->type == IHM_MSG_CONFIRM_LIMIT_ENTR) {
        if (ihm_state.curr_page == 2)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_LIMIT_M1) {
        if (ihm_state.curr_page == 3)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_LIMIT_M2) {
        if (ihm_state.curr_page == 4)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_LIMIT_M3) {
        if (ihm_state.curr_page == 5)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_LIMIT_M4) {
        if (ihm_state.curr_page == 6)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_IS_AWARE_ENTR) {
        if (ihm_state.curr_page == 7 || ihm_state.curr_page == 8)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_IS_AWARE_M1) {
        if (ihm_state.curr_page == 9 || ihm_state.curr_page == 10)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_IS_AWARE_M2) {
        if (ihm_state.curr_page == 11 || ihm_state.curr_page == 12)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_IS_AWARE_M3) {
        if (ihm_state.curr_page == 13 || ihm_state.curr_page == 14)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CONFIRM_IS_AWARE_M4) {
        if (ihm_state.curr_page == 15 || ihm_state.curr_page == 16)
            write_change_page(1);
    } else if (update_event->type == IHM_MSG_CHANGE_CONNECT) {
        if (ihm_state.curr_page == 1)
            write_pic_id(update_event->payload, 30);
    } else if (update_event->type == IHM_MSG_CHANGE_DISCONNECT) {
        if (ihm_state.curr_page == 1) {
            write_pic_id(update_event->payload, 29);
            write_text_temperature(update_event->payload, -1);
        }
    } else if (update_event->type == IHM_MSG_CHANGE_QUEIMADOR_MODE) {
        if (ihm_state.curr_page == 1)
            write_queimador_mode(update_event->payload);
    } else if (update_event->type == IHM_MSG_NOTIFY_ENTR_STATE) {
        int state = update_event->payload;
        if (state == -1) {
            if (ihm_state.curr_page != 7)
                write_change_page(7);
        } else if (state == 1) {
            if (ihm_state.curr_page != 8)
                write_change_page(8);
        }
    } else if (update_event->type == IHM_MSG_NOTIFY_M1_STATE) {
        int state = update_event->payload;
        if (state == -1) {
            if (ihm_state.curr_page != 9)
                write_change_page(9);
        } else if (state == 1) {
            if (ihm_state.curr_page != 10)
                write_change_page(10);
        }
    } else if (update_event->type == IHM_MSG_NOTIFY_M2_STATE) {
        int state = update_event->payload;
        if (state == -1) {
            if (ihm_state.curr_page != 11)
                write_change_page(11);
        } else if (state == 1) {
            if (ihm_state.curr_page != 12)
                write_change_page(12);
        }
    } else if (update_event->type == IHM_MSG_NOTIFY_M3_STATE) {
        int state = update_event->payload;
        if (state == -1) {
            if (ihm_state.curr_page != 13)
                write_change_page(13);
        } else if (state == 1) {
            if (ihm_state.curr_page != 14)
                write_change_page(14);
        }
    } else if (update_event->type == IHM_MSG_NOTIFY_M4_STATE) {
        int state = update_event->payload;
        if (state == -1) {
            if (ihm_state.curr_page != 15)
                write_change_page(15);
        } else if (state == 1) {
            if (ihm_state.curr_page != 16)
                write_change_page(16);
        }
    }
}

static void process_update(IHMMessage_t *update_event) {
    switch (ihm_state.curr_state) {
        case STARTING:
            handle_update_starting(update_event);
            break;

        case RUNNING:
            handle_update_running(update_event);
            break;
    }
}

static void ihm_task(void *pvParameters) {
    QueueHandle_t queue_handle;

    uart_event_t input_event;
    IHMMessage_t update_event;

    for (;;) {
        queue_handle = (QueueHandle_t)xQueueSelectFromSet(ihm_qs, portMAX_DELAY);

        if (queue_handle == ihm_input_q) {
            xQueueReceive(ihm_input_q, (void *)&input_event, (TickType_t)0);
            process_input(&input_event);
        } else if (queue_handle == ihm_msg_q) {
            xQueueReceive(ihm_msg_q, (void *)&update_event, (TickType_t)0);
            process_update(&update_event);
        }
    }
}

void ihm_controller_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &ihm_input_q, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ihm_qs = xQueueCreateSet(IHM_QUEUE_SET_LEN);
    xQueueAddToSet(ihm_input_q, ihm_qs);
    xQueueAddToSet(ihm_msg_q, ihm_qs);

    xTaskCreate(ihm_task, "IHM_INPUT_TASK", 6000, NULL, 3, NULL);
}
