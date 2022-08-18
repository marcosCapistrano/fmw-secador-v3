#include "controller.h"

#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "storage.h"
#include "common.h"

#define PIN_QUEIMADOR 12
#define PIN_ALARME 13
#define PIN_MASSA_QUENTE 4
#define PIN_MASSA_FRIO 5
#define PIN_ENTRADA_QUENTE 2
#define PIN_ENTRADA_FRIO 0
#define PIN_CONEXAO 18
#define PIN_SENSOR 15

#define GPIO_OUTPUT_SEL ((1ULL << PIN_QUEIMADOR) | (1ULL << PIN_ALARME) | (1ULL << PIN_MASSA_QUENTE) | (1ULL << PIN_MASSA_FRIO) | (1ULL << PIN_ENTRADA_QUENTE) | (1ULL << PIN_ENTRADA_FRIO) | (1ULL << PIN_CONEXAO))
#define GPIO_INPUT_SEL (1ULL << PIN_SENSOR)

extern QueueHandle_t ihm_update_q;

static void set_queimador(bool on);
static void set_alarme(bool on);
static void set_led_entr_quente(bool on);
static void set_led_entr_frio(bool on);
static void set_led_mass_quente(bool on);
static void set_led_mass_frio(bool on);
static void set_led_conexao(bool on);

void controller_init(void) {
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

    // Inicializar leitor sensor físico
    

    // Notificar LCD
    IHMUpdate_t ihm_update = {.type = IHM_UPDATE_TYPE_INIT, .payload = (void *)IHM_UPDATE_INIT_SENSOR_ENTR};
    xQueueSend(ihm_update_q, &ihm_update, portMAX_DELAY);
    // Salvar horário de ligamento
}

void controller_task(void *pvParameters) {
    /*
        This task will receive events from:
        - IHM input - for page/state update
        - Server - for state update
    */
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