#include "perif_controller.h"

#include <stdbool.h>

#include "common.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "ds18b20.h"
#include "esp_log.h"
#include "owb.h"
#include "owb_rmt.h"
#include "storage.h"

#define PIN_QUEIMADOR 21
#define PIN_ALARME 22
#define PIN_MASSA_QUENTE 4
#define PIN_MASSA_FRIO 5
#define PIN_ENTRADA_QUENTE 2
#define PIN_ENTRADA_FRIO 0
#define PIN_CONEXAO 18
#define PIN_SENSOR 19

#define GPIO_OUTPUT_SEL ((1ULL << PIN_QUEIMADOR) | (1ULL << PIN_ALARME) | (1ULL << PIN_MASSA_QUENTE) | (1ULL << PIN_MASSA_FRIO) | (1ULL << PIN_ENTRADA_QUENTE) | (1ULL << PIN_ENTRADA_FRIO) | (1ULL << PIN_CONEXAO))
#define GPIO_INPUT_SEL (1ULL << PIN_SENSOR)

#define GPIO_DS18B20_0 (CONFIG_ONE_WIRE_GPIO)
#define MAX_DEVICES (8)
#define DS18B20_RESOLUTION (DS18B20_RESOLUTION_12_BIT)
#define SAMPLE_PERIOD (1000)  // milliseconds

extern QueueHandle_t perif_msg_q;

static void set_queimador(bool on);
static void set_alarme(bool on);
static void set_led_entr_quente(bool on);
static void set_led_entr_frio(bool on);
static void set_led_mass_quente(bool on);
static void set_led_mass_frio(bool on);
static void set_led_conexao(bool on);

/*
    This task will receive events from:
    - IHM input - for page/state update
    - Server - for state update
*/
static void perif_controller_task(void *pvParameters) {
    OneWireBus *owb;
    owb_rmt_driver_info rmt_driver_info;
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true); // enable CRC check for ROM code

    DS18B20_Info *device = 0;
    DS18B20_Info *ds18b20_info = ds18b20_malloc();  // heap allocation
    device = ds18b20_info;
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

    PerifMessage_t perif_msg;

    for (;;) {
        if (xQueueReceive(perif_msg_q, &perif_msg, pdMS_TO_TICKS(10000))) {

        }

        ds18b20_convert_all(owb);
        ds18b20_wait_for_conversion(device);

        float reading = 0;
        DS18B20_ERROR error = ds18b20_read_temp(device, &reading);

        common_send_ihm_msg(IHM_MSG_CHANGE_SENSOR_ENTR, (void *)(int)reading, portMAX_DELAY);
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

    xTaskCreate(perif_controller_task, "PERIF_CONTROLLER_TASK", 2400, NULL, 5, NULL);
}

static void set_queimador(bool on) {
    if (on) {
        gpio_set_level(PIN_QUEIMADOR, 1);
    } else {
        gpio_set_level(PIN_QUEIMADOR, 0);
    }
}

static void set_alarme(bool on) {
    if (on) {
        gpio_set_level(PIN_ALARME, 1);
    } else {
        gpio_set_level(PIN_ALARME, 0);
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