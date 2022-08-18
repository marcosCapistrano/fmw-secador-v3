#include <stdbool.h>
#include "controller.h"

#include "driver/gpio.h"

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

void controller_init(void) {
    gpio_config_t gpio_conf = {};
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_OUTPUT;
    gpio_conf.pin_bit_mask = GPIO_OUTPUT_SEL;
    gpio_conf.pull_down_en = 0;
    gpio_conf.pull_up_en = 0;
    gpio_config(&gpio_conf);

    // Desligar todos os periféricos
    // Inicializar leitor sensor físico
    // Esperar confirmação do storage para poder iniciar
}

void controller_task(void *pvParameters) {
    /*
        This task will receive events from:
        - IHM input - for page/state update
        - Server - for state update
    */
}
