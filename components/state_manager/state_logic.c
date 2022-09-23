#include "state_logic.h"

#include "common.h"
#include "storage.h"

#define HIST 7

/*
    Pegar variaveis no storage, e retornar as devidas ações apropriadas
    Deverá rodar logica de:

    - Queimador
    - Alarme
    - Led Entrada Quente
    - Led Entrada Frio
    - Led Massa Quente
    - Led Massa Frio
    - Led Conexao
*/

/*
    Para decidir como atuar sobre o alarme, precisamos:
    - Estado atual do alarme (on/off)
    - Valores atuais dos sensores
    - Saber se o operador já silenciou algum dos sensores
*/

bool is_entr_within_limits() {
    int sensor_entr = storage_get_sensor_entr();

    int limit_min_entr = storage_get_min_entr();
    int limit_max_entr = storage_get_max_entr();
    return (sensor_entr > limit_min_entr && sensor_entr < limit_min_entr);
}

bool is_entr_within_limits_hist() {
    int sensor_entr = storage_get_sensor_entr();

    int limit_min_entr = storage_get_min_entr();
    int limit_max_entr = storage_get_max_entr();
    return (sensor_entr > (limit_min_entr - HIST) && sensor_entr < (limit_min_entr + HIST));
}

bool is_entr_lower() {
    int sensor_entr = storage_get_sensor_entr();
    int limit_min_entr = storage_get_min_entr();

    return sensor_entr < limit_min_entr;
}

bool is_entr_lower_hist() {
    int sensor_entr = storage_get_sensor_entr();
    int limit_min_entr = storage_get_min_entr();

    return sensor_entr < (limit_min_entr - HIST);
}

bool is_entr_higher() {
    int sensor_entr = storage_get_sensor_entr();
    int limit_max_entr = storage_get_max_entr();

    return sensor_entr > limit_max_entr;
}

bool is_entr_higher_hist() {
    int sensor_entr = storage_get_sensor_entr();
    int limit_max_entr = storage_get_max_entr();

    return sensor_entr > (limit_max_entr + HIST);
}

bool is_m1_within_limits() {
    int sensor_m1 = storage_get_sensor_m1();

    int limit_min_m1 = storage_get_min_m1();
    int limit_max_m1 = storage_get_max_m1();
    return (sensor_m1 > limit_min_m1 && sensor_m1 < limit_min_m1);
}

bool is_m1_within_limits_hist() {
    int sensor_m1 = storage_get_sensor_m1();

    int limit_min_m1 = storage_get_min_m1();
    int limit_max_m1 = storage_get_max_m1();
    return (sensor_m1 > (limit_min_m1 - HIST) && sensor_m1 < (limit_min_m1 + HIST));
}

bool is_m1_lower() {
    int sensor_m1 = storage_get_sensor_m1();
    int limit_min_m1 = storage_get_min_m1();

    return sensor_m1 < limit_min_m1;
}

bool is_m1_lower_hist() {
    int sensor_m1 = storage_get_sensor_m1();
    int limit_min_m1 = storage_get_min_m1();

    return sensor_m1 < (limit_min_m1 - HIST);
}

bool is_m1_higher() {
    int sensor_m1 = storage_get_sensor_m1();
    int limit_max_m1 = storage_get_max_m1();

    return sensor_m1 > limit_max_m1;
}

bool is_m1_higher_hist() {
    int sensor_m1 = storage_get_sensor_m1();
    int limit_max_m1 = storage_get_max_m1();

    return sensor_m1 > (limit_max_m1 + HIST);
}

bool is_m2_within_limits() {
    int sensor_m2 = storage_get_sensor_m2();

    int limit_min_m2 = storage_get_min_m2();
    int limit_max_m2 = storage_get_max_m2();
    return (sensor_m2 > limit_min_m2 && sensor_m2 < limit_min_m2);
}

bool is_m2_within_limits_hist() {
    int sensor_m2 = storage_get_sensor_m2();

    int limit_min_m2 = storage_get_min_m2();
    int limit_max_m2 = storage_get_max_m2();
    return (sensor_m2 > (limit_min_m2 - HIST) && sensor_m2 < (limit_min_m2 + HIST));
}

bool is_m2_lower() {
    int sensor_m2 = storage_get_sensor_m2();
    int limit_min_m2 = storage_get_min_m2();

    return sensor_m2 < limit_min_m2;
}

bool is_m2_lower_hist() {
    int sensor_m2 = storage_get_sensor_m2();
    int limit_min_m2 = storage_get_min_m2();

    return sensor_m2 < (limit_min_m2 - HIST);
}

bool is_m2_higher() {
    int sensor_m2 = storage_get_sensor_m2();
    int limit_max_m2 = storage_get_max_m2();

    return sensor_m2 > limit_max_m2;
}

bool is_m2_higher_hist() {
    int sensor_m2 = storage_get_sensor_m2();
    int limit_max_m2 = storage_get_max_m2();

    return sensor_m2 > (limit_max_m2 + HIST);
}

bool is_m3_within_limits() {
    int sensor_m3 = storage_get_sensor_m3();

    int limit_min_m3 = storage_get_min_m3();
    int limit_max_m3 = storage_get_max_m3();
    return (sensor_m3 > limit_min_m3 && sensor_m3 < limit_min_m3);
}

bool is_m3_within_limits_hist() {
    int sensor_m3 = storage_get_sensor_m3();

    int limit_min_m3 = storage_get_min_m3();
    int limit_max_m3 = storage_get_max_m3();
    return (sensor_m3 > (limit_min_m3 - HIST) && sensor_m3 < (limit_min_m3 + HIST));
}

bool is_m3_lower() {
    int sensor_m3 = storage_get_sensor_m3();
    int limit_min_m3 = storage_get_min_m3();

    return sensor_m3 < limit_min_m3;
}

bool is_m3_lower_hist() {
    int sensor_m3 = storage_get_sensor_m3();
    int limit_min_m3 = storage_get_min_m3();

    return sensor_m3 < (limit_min_m3 - HIST);
}

bool is_m3_higher() {
    int sensor_m3 = storage_get_sensor_m3();
    int limit_max_m3 = storage_get_max_m3();

    return sensor_m3 > limit_max_m3;
}

bool is_m3_higher_hist() {
    int sensor_m3 = storage_get_sensor_m3();
    int limit_max_m3 = storage_get_max_m3();

    return sensor_m3 > (limit_max_m3 + HIST);
}

bool is_m4_within_limits() {
    int sensor_m4 = storage_get_sensor_m4();

    int limit_min_m4 = storage_get_min_m4();
    int limit_max_m4 = storage_get_max_m4();
    return (sensor_m4 > limit_min_m4 && sensor_m4 < limit_min_m4);
}

bool is_m4_within_limits_hist() {
    int sensor_m4 = storage_get_sensor_m4();

    int limit_min_m4 = storage_get_min_m4();
    int limit_max_m4 = storage_get_max_m4();
    return (sensor_m4 > (limit_min_m4 - HIST) && sensor_m4 < (limit_min_m4 + HIST));
}

bool is_m4_lower() {
    int sensor_m4 = storage_get_sensor_m4();
    int limit_min_m4 = storage_get_min_m4();

    return sensor_m4 < limit_min_m4;
}

bool is_m4_lower_hist() {
    int sensor_m4 = storage_get_sensor_m4();
    int limit_min_m4 = storage_get_min_m4();

    return sensor_m4 < (limit_min_m4 - HIST);
}

bool is_m4_higher() {
    int sensor_m4 = storage_get_sensor_m4();
    int limit_max_m4 = storage_get_max_m4();

    return sensor_m4 > limit_max_m4;
}

bool is_m4_higher_hist() {
    int sensor_m4 = storage_get_sensor_m4();
    int limit_max_m4 = storage_get_max_m4();

    return sensor_m4 > (limit_max_m4 + HIST);
}

PerifMessage_t get_perif_alarme_action() {
    bool alarme_state = storage_get_alarme_state();

    bool is_aware_entr = storage_get_is_aware_entr();
    bool is_aware_m1 = storage_get_is_aware_m1();
    bool is_aware_m2 = storage_get_is_aware_m2();
    bool is_aware_m3 = storage_get_is_aware_m3();
    bool is_aware_m4 = storage_get_is_aware_m4();

    /*
        Se o alarme estiver ligado, ele deverá desligar quando:
        - Todos os sensores estiverem dentro das faixas apropriadas
        - Quando os sensores que estiverem fora da faixa possuem is_aware verdadeiro
    */
    if (alarme_state) {
        bool should_turn_off = false;

        if () &&(sensor_entr > (limit_min_entr - HIST) && sensor_entr < (limit_min_entr + HIST))
    }
}

// pegar variaveis no storage, e apartir delas decidir se o alarme deve ser ligada ou desligado
PerifMessage_t get_perif_queimador_action() {
}

IHMMessage_t get_ihm_action() {
}