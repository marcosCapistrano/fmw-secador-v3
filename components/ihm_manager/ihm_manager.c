#include "ihm_manager.h"

#include "common.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define UART_NUM (UART_NUM_2)
#define TXD_PIN 17
#define RXD_PIN 16

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

#define IHM_INPUT_QUEUE_LEN 5
#define IHM_UPDATE_QUEUE_LEN 5
#define QUEUE_SET_LEN (IHM_INPUT_QUEUE_LEN + IHM_UPDATE_QUEUE_LEN)

static QueueSetHandle_t ihm_qs;
static QueueHandle_t ihm_input_q;
extern QueueHandle_t ihm_update_q;

static void ihm_task(void *pvParameters);

static const char *TAG = "IHM_MANAGER";

enum IHMPage_t {
    INIT,
    SENSORS
};

typedef struct {
    bool storage;
    bool sensor_entr;
    bool sensor_m1;
    bool sensor_m2;
    bool sensor_m3;
    bool sensor_m4;
} IHMInitConditions_t;

typedef struct {
    enum IHMPage_t curr_page;
    IHMInitConditions_t init_conditions;
} IHMState_t;

static IHMState_t ihm_state;

void ihm_manager_init(void) {
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

    ihm_update_q = xQueueCreate(IHM_UPDATE_QUEUE_LEN, sizeof(IHMUpdate_t));

    ihm_qs = xQueueCreateSet(QUEUE_SET_LEN);
    xQueueAddToSet(ihm_input_q, ihm_qs);
    xQueueAddToSet(ihm_update_q, ihm_qs);

    ihm_state = (IHMState_t){
        .curr_page = INIT,
    };

    /*
        Wait for confirmation from other components, change to sensors screen, and then start the task.
    */

    xTaskCreate(ihm_task, "IHM_INPUT_TASK", 6000, NULL, 3, NULL);
}

static void process_input(uart_event_t *input_event) {
    static int length = 0;
    static uint8_t data[128];

    ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, (size_t *)&length));
    length = uart_read_bytes(UART_NUM, data, length, portMAX_DELAY);

    for (int i = 0; i < length; i++) {
        ESP_LOGI("OI", "Received: %d", data[i]);
    }

    if (ihm_state.curr_page == INIT) {
        // Do nothing
    }

    // PROCESS AND DISPATCH TO main_controller
    // uart_flush(UART_NUM);
}

static void process_init_conditions(IHMUpdateTypeInit_t update_type) {
}

static void process_update(IHMUpdate_t update_event) {
    ESP_LOGI(TAG, "Received Init condition: %d", (IHMUpdateTypeInit_t)update_event.payload);
    switch (ihm_state.curr_page) {
        case INIT: {
            if (update_event.type == IHM_UPDATE_TYPE_INIT) {
                ESP_LOGI(TAG, "Received Init condition: %d", (IHMUpdateTypeInit_t)update_event.payload);
                process_init_conditions((IHMUpdateTypeInit_t)update_event.payload);
                break;
            }
        }

        case SENSORS: {
            break;
        }
    }
}

static void ihm_task(void *pvParameters) {
    QueueHandle_t queue_handle;

    uart_event_t *input_event;
    IHMUpdate_t update_event;

    for (;;) {
        ESP_LOGI(TAG, "Oi");
        queue_handle = (QueueHandle_t)xQueueSelectFromSet(ihm_qs, portMAX_DELAY);
        ESP_LOGI(TAG, "XAu");

        if (queue_handle == ihm_input_q) {
            xQueueReceive(ihm_input_q, (void *)&input_event, (TickType_t)0);
            process_input(input_event);
        } else if (queue_handle == ihm_update_q) {
            xQueueReceive(ihm_update_q, (void *)&update_event, (TickType_t)0);
            process_update(update_event);
        }
    }
}