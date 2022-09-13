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

typedef struct {
    uint8_t curr_page;
} IHMState_t;

static IHMState_t ihm_state = {
    .curr_page = 0};

static QueueSetHandle_t ihm_qs;
static QueueHandle_t ihm_input_q;
extern QueueHandle_t ihm_msg_q;
extern QueueHandle_t state_msg_q;

static const char *TAG = "IHM_MANAGER";

static void write_to_ihm(const char *command) {
    size_t length = strlen(command);

    uart_write_bytes(UART_NUM, "\xFF\xFF\xFF", 3);
    uart_write_bytes(UART_NUM, command, length);
    uart_write_bytes(UART_NUM, "\xFF\xFF\xFF", 3);
}

static void write_text_temperature(int tNumber, int value) {
    char temp_buf[25] = {0};

    nex_text_change_temp(temp_buf, tNumber, value);
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
        write_button_pic_id(5, 24);
    } else {
        write_button_pic_id(5, 29);
    }
}

static void dispatch_button_released(uint8_t page_id, uint8_t component_id) {
    if (page_id == 1) {           // Pagina sensores
        if (component_id == 7) {  // Entrada
            write_change_page(2);
        } else if (component_id == 8) {  // Massa 1
            write_change_page(3);
        } else if (component_id == 9) {  // Massa 2
            write_change_page(4);
        } else if (component_id == 10) {  // Massa 3
            write_change_page(5);
        } else if (component_id == 11) {  // Massa 4
            write_change_page(6);
        } else if (component_id == 1) {  // Palha Lenha
            if (storage_get_queimador_mode()) {  // Se tiver na lenha
                common_send_state_msg(STA_MSG_CHANGE_QUEIMADOR_MODE, (void *)false, portMAX_DELAY);
            } else {  // Se tiver na palha
                common_send_state_msg(STA_MSG_CHANGE_QUEIMADOR_MODE, (void *)true, portMAX_DELAY);
            }
        }
    } else if (page_id == 2) {    // Pagina Limit Entr
        if (component_id == 7) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 8) {  // Aplicar
        }
    } else if (page_id == 3) {    // Pagina Limit M1
        if (component_id == 7) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 8) {  // Aplicar
        }
    } else if (page_id == 4) {  // Pagina Limit M2

        if (component_id == 7) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 8) {  // Aplicar
        }
    } else if (page_id == 5) {  // Pagina Limit M3

        if (component_id == 7) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 8) {  // Aplicar
        }
    } else if (page_id == 6) {    // Pagina Limit M4
        if (component_id == 7) {  // Cancelar
            write_change_page(1);
        } else if (component_id == 8) {  // Aplicar
        }
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
            write_text_temperature(1, sensor_m1);
            write_text_temperature(2, sensor_m2);
            write_text_temperature(3, sensor_m3);
            write_text_temperature(4, sensor_m4);

            write_queimador_mode(queimador_mode);
        } break;
    }
}

static void process_input(uart_event_t *uart_event) {
    size_t buffer_size = uart_event->size;
    uint8_t data[128];

    /*
    Checa se o tipo de evento é do tipo 0 (DATA), se não for, analisar pq pode ser fonte de bugs
    */
    if (uart_event->type != 0) {
        ESP_LOGE(TAG, "Uart Event Type returned: %d", uart_event->type);
    }

    uart_read_bytes(UART_NUM, data, buffer_size, portMAX_DELAY);
    for (int i = 0; i < buffer_size; i++) {
        ESP_LOGI(TAG, "[%d]: %d", i, data[i]);
    }

    uint8_t data_head = data[0];
    uint8_t data_page_id = data[1];

    if (data_head == 101) {
        uint8_t data_component_id = data[2];
        dispatch_button_released(data_page_id, data_component_id);
    } else if (data_head == 102) {
        dispatch_page_loaded(data_page_id);
    }
}

static void process_update(IHMMessage_t *update_event) {
    switch (update_event->type) {
        case IHM_MSG_CHANGE_QUEIMADOR_MODE:
            if (ihm_state.curr_page == 1) {
                ESP_LOGE(TAG, "Payload: %d", (int) update_event->payload);
                write_queimador_mode(update_event->payload);
            }
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
            ESP_LOGI(TAG, "Received from update q");
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

    write_change_page(1);
    xTaskCreate(ihm_task, "IHM_INPUT_TASK", 6000, NULL, 3, NULL);
}
