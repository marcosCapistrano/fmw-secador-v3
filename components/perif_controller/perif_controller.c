#include "perif_controller.h"

#include <stdbool.h>

#include "common.h"
#include "driver/gpio.h"
#include "ds18b20.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "owb.h"
#include "owb_rmt.h"
#include "storage.h"

#define PIN_QUEIMADOR 32
#define PIN_ALARME 33
#define PIN_MASSA_QUENTE 25
#define PIN_MASSA_FRIO 26
#define PIN_ENTRADA_QUENTE 27
#define PIN_ENTRADA_FRIO 12
#define PIN_CONEXAO 13
#define PIN_SENSOR 19

#define GPIO_OUTPUT_SEL ((1ULL << PIN_QUEIMADOR) | (1ULL << PIN_ALARME))
#define GPIO_INPUT_SEL (1ULL << PIN_SENSOR)

#define GPIO_DS18B20_0 (CONFIG_ONE_WIRE_GPIO)
#define MAX_DEVICES (8)
#define DS18B20_RESOLUTION (DS18B20_RESOLUTION_12_BIT)
#define SAMPLE_PERIOD (1000)  // milliseconds

static OneWireBus *owb = 0;
static DS18B20_Info *ds18b20_info = 0;
static owb_rmt_driver_info rmt_driver_info;

static const char *TAG = "PERIF_CONTROLLER";

typedef enum {
    STARTING,
    RUNNING,
} State_t;

static State_t curr_state = STARTING;

extern QueueHandle_t perif_msg_q;

static void set_queimador(bool on);
static void set_alarme(bool on);
static void set_led_entr_quente(bool on);
static void set_led_entr_frio(bool on);
static void set_led_mass_quente(bool on);
static void set_led_mass_frio(bool on);
static void set_led_conexao(bool on);

static void handle_starting(PerifMessage_t *perif_msg) {
    if (perif_msg->type == PERIF_MSG_RUN) {
        curr_state = RUNNING;
    } else if (perif_msg->type == PERIF_MSG_FINISH) {
        curr_state = STARTING;
    }
}

static bool should_other_alarm_run(int sensor_id) { //0 = entrada, 1 = m1...
    bool alarme_entr = storage_get_should_alarme_entr();
    bool alarme_m1 = storage_get_should_alarme_m1();
    bool alarme_m2 = storage_get_should_alarme_m2();
    bool alarme_m3 = storage_get_should_alarme_m3();
    bool alarme_m4 = storage_get_should_alarme_m4();

    if(sensor_id == 0) {
        return (alarme_m1 || alarme_m2 || alarme_m3 || alarme_m4);
    } else if(sensor_id == 1) {
        return (alarme_entr || alarme_m2 || alarme_m3 || alarme_m4);
    } else if(sensor_id == 2) {
        return (alarme_entr || alarme_m1 || alarme_m3 || alarme_m4);
    } else if(sensor_id == 3) {
        return (alarme_entr || alarme_m1 || alarme_m2 || alarme_m4);
    } else if(sensor_id == 4) {
        return (alarme_entr || alarme_m1 || alarme_m2 || alarme_m3);
    }

    return false;
}

static bool should_other_queimador_stop(int sensor_id) { //0 = entrada, 1 = m1...
    bool queimador_entr = storage_get_should_queimador_entr();
    bool queimador_m1 = storage_get_should_queimador_m1();
    bool queimador_m2 = storage_get_should_queimador_m2();
    bool queimador_m3 = storage_get_should_queimador_m3();
    bool queimador_m4 = storage_get_should_queimador_m4();

    if(sensor_id == 0) {
        return (!queimador_m1 || !queimador_m2 || !queimador_m3 || !queimador_m4);
    } else if(sensor_id == 1) {
        return (!queimador_entr || !queimador_m2 || !queimador_m3 || !queimador_m4);
    } else if(sensor_id == 2) {
        return (!queimador_entr || !queimador_m1 || !queimador_m3 || !queimador_m4);
    } else if(sensor_id == 3) {
        return (!queimador_entr || !queimador_m1 || !queimador_m2 || !queimador_m4);
    } else if(sensor_id == 4) {
        return (!queimador_entr || !queimador_m1 || !queimador_m2 || !queimador_m3);
    }

    return false;
}

static void handle_running(PerifMessage_t *perif_msg) {
    if (perif_msg->type == PERIF_MSG_NOTIFY_LOW_ENTR) {
        if(!storage_get_alarme_state()) {
            set_alarme(true);
        }

        if(!should_other_queimador_stop(0) && !storage_get_queimador_state()) {
            set_queimador(true);
        }
    } else if (perif_msg->type == PERIF_MSG_NOTIFY_NORMAL_ENTR) {
        if(!should_other_alarm_run(0) && storage_get_alarme_state()) {
            set_alarme(false);
        }

        if(!should_other_queimador_stop(0) && !storage_get_queimador_state()) {
            set_queimador(true);
        }
    } else if (perif_msg->type == PERIF_MSG_NOTIFY_HIGH_ENTR) {
        if(!storage_get_alarme_state()) {
            set_alarme(true);
        }

        if(storage_get_queimador_state()) {
            set_queimador(false);
        }
    } else if (perif_msg->type == PERIF_MSG_NOTIFY_LOW_MASS) {
        if(!storage_get_alarme_state()) {
            set_alarme(true);
        }

        if(!should_other_queimador_stop(perif_msg->payload) && !storage_get_queimador_state()) {
            set_queimador(true);
        }
    } else if (perif_msg->type == PERIF_MSG_NOTIFY_NORMAL_MASS) {
        if(!should_other_alarm_run(perif_msg->payload) && storage_get_alarme_state()) {
            set_alarme(false);
        }

        if(!should_other_queimador_stop(perif_msg->payload) && !storage_get_queimador_state()) {
            set_queimador(true);
        }
    } else if (perif_msg->type == PERIF_MSG_NOTIFY_HIGH_MASS) {
        if(!storage_get_alarme_state()) {
            set_alarme(true);
        }

        if(storage_get_queimador_state()) {
            set_queimador(false);
        }
    } else if (perif_msg->type == PERIF_MSG_NOTIFY_CONNECTED) {
    } else if (perif_msg->type == PERIF_MSG_NOTIFY_DISCONNECTED) {
    }
}

/*
    This task will receive events from:
    - IHM input - for page/state update
    - Server - for state update
*/

static void sensor_temp_init() {
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true);  // enable CRC check for ROM code

    ds18b20_info = ds18b20_malloc();
    ds18b20_init_solo(ds18b20_info, owb);

    ds18b20_use_crc(ds18b20_info, true);  // enable CRC check on all reads
    ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);

    bool parasitic_power = false;
    ds18b20_check_for_parasite_power(owb, &parasitic_power);
    if (parasitic_power) {
        printf("Parasitic-powered devices detected");
    }

    // In parasitic-power mode, devices cannot indicate when conversions are complete,
    // so waiting for a temperature conversion must be done by waiting a prescribed duration
    owb_use_parasitic_power(owb, parasitic_power);
}

static int sensor_temp_read() {
    ds18b20_convert_all(owb);
    ds18b20_wait_for_conversion(ds18b20_info);

    float reading = 0;
    DS18B20_ERROR error = ds18b20_read_temp(ds18b20_info, &reading);

    return ((int)reading);
}

static void perif_controller_task(void *pvParameters) {
    PerifMessage_t perif_msg;

    for (;;) {
        if (xQueueReceive(perif_msg_q, &perif_msg, pdMS_TO_TICKS(15000))) {
            switch (curr_state) {
                case STARTING:
                    handle_starting(&perif_msg);
                    break;

                case RUNNING:
                    handle_running(&perif_msg);
                    break;
            }
        }

        if (curr_state == RUNNING) {
            int last_sensor_entr = storage_get_sensor_entr();
            int sensor_entr = sensor_temp_read();

            if (last_sensor_entr != sensor_entr)
                common_send_state_msg(STA_MSG_CHANGE_SENSOR_ENTR, (void *)sensor_entr, portMAX_DELAY);
        }
    }
}

void perif_controller_init(void) {
    gpio_config_t gpio_conf = {};
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_OUTPUT;
    gpio_conf.pin_bit_mask = GPIO_OUTPUT_SEL;
    gpio_conf.pull_down_en = 0;
    gpio_conf.pull_up_en = 0;
    gpio_config(&gpio_conf);

    // Desligar todos os periféricos
    set_queimador(false);
    set_alarme(false);
    set_led_mass_quente(false);
    set_led_mass_frio(false);
    set_led_entr_quente(false);
    set_led_entr_frio(false);
    set_led_conexao(false);

    sensor_temp_init();
    xTaskCreate(perif_controller_task, "PERIF_CONTROLLER_TASK", 2400, NULL, 5, NULL);
}

static void set_queimador(bool on) {
    if (on) {
        gpio_set_level(PIN_QUEIMADOR, 1);
        storage_set_queimador_state(true);
    } else {
        gpio_set_level(PIN_QUEIMADOR, 0);
        storage_set_queimador_state(false);
    }
}

static void set_alarme(bool on) {
    if (on) {
        gpio_set_level(PIN_ALARME, 1);
        storage_set_alarme_state(true);
    } else {
        gpio_set_level(PIN_ALARME, 0);
        storage_set_alarme_state(false);
    }
}

static void set_led_entr_quente(bool on) {
    if (on) {
        gpio_set_level(PIN_ENTRADA_QUENTE, 0);
    } else {
        gpio_set_level(PIN_ENTRADA_QUENTE, 1);
    }
}

static void set_led_entr_frio(bool on) {
    if (on) {
        gpio_set_level(PIN_ENTRADA_FRIO, 0);
    } else {
        gpio_set_level(PIN_ENTRADA_FRIO, 1);
    }
}

static void set_led_mass_quente(bool on) {
    if (on) {
        gpio_set_level(PIN_MASSA_QUENTE, 0);
    } else {
        gpio_set_level(PIN_MASSA_QUENTE, 1);
    }
}

static void set_led_mass_frio(bool on) {
    if (on) {
        gpio_set_level(PIN_MASSA_FRIO, 0);
    } else {
        gpio_set_level(PIN_MASSA_FRIO, 1);
    }
}

static void set_led_conexao(bool on) {
    if (on) {
        gpio_set_level(PIN_CONEXAO, 1);
    } else {
        gpio_set_level(PIN_CONEXAO, 0);
    }
}