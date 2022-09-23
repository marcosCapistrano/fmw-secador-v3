#include "state_logic.h"

#include "common.h"
#include "esp_log.h"
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
    return (sensor_entr > limit_min_entr && sensor_entr < limit_max_entr);
}

bool is_entr_within_limits_hist() {
    int sensor_entr = storage_get_sensor_entr();

    int limit_min_entr = storage_get_min_entr();
    int limit_max_entr = storage_get_max_entr();
    return (sensor_entr > (limit_min_entr - HIST) && sensor_entr < (limit_max_entr + HIST));
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

    return (sensor_entr > (limit_max_entr + HIST));
}

bool is_m1_within_limits() {
    int sensor_m1 = storage_get_sensor_m1();

    int limit_min_m1 = storage_get_min_m1();
    int limit_max_m1 = storage_get_max_m1();
    return (sensor_m1 > limit_min_m1 && sensor_m1 < limit_max_m1);
}

bool is_m1_within_limits_hist() {
    int sensor_m1 = storage_get_sensor_m1();

    int limit_min_m1 = storage_get_min_m1();
    int limit_max_m1 = storage_get_max_m1();
    return (sensor_m1 > (limit_min_m1 - HIST) && sensor_m1 < (limit_max_m1 + HIST));
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
    return (sensor_m2 > limit_min_m2 && sensor_m2 < limit_max_m2);
}

bool is_m2_within_limits_hist() {
    int sensor_m2 = storage_get_sensor_m2();

    int limit_min_m2 = storage_get_min_m2();
    int limit_max_m2 = storage_get_max_m2();
    return (sensor_m2 > (limit_min_m2 - HIST) && sensor_m2 < (limit_max_m2 + HIST));
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
    return (sensor_m3 > limit_min_m3 && sensor_m3 < limit_max_m3);
}

bool is_m3_within_limits_hist() {
    int sensor_m3 = storage_get_sensor_m3();

    int limit_min_m3 = storage_get_min_m3();
    int limit_max_m3 = storage_get_max_m3();
    return (sensor_m3 > (limit_min_m3 - HIST) && sensor_m3 < (limit_max_m3 + HIST));
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
    return (sensor_m4 > limit_min_m4 && sensor_m4 < limit_max_m4);
}

bool is_m4_within_limits_hist() {
    int sensor_m4 = storage_get_sensor_m4();

    int limit_min_m4 = storage_get_min_m4();
    int limit_max_m4 = storage_get_max_m4();
    return (sensor_m4 > (limit_min_m4 - HIST) && sensor_m4 < (limit_max_m4 + HIST));
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

PerifMessageType_t get_perif_alarme_action(void **payload) {
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

    ESP_LOGE("STATE_LOGIC", "Entr: %u, M1: %u, M2: %u, M3: %u, M4: %u", storage_get_sensor_entr(), storage_get_sensor_m1(), storage_get_sensor_m2(), storage_get_sensor_m3(), storage_get_sensor_m4());
    if (alarme_state) {
        if (is_entr_within_limits_hist() &&
            is_m1_within_limits_hist() &&
            is_m2_within_limits_hist() &&
            is_m3_within_limits_hist() &&
            is_m4_within_limits_hist()) {
            *payload = (void *)false;
            return PERIF_MSG_NOTIFY_ALARME_STATE;
        } else {
            if (!is_entr_within_limits_hist() && !is_aware_entr) {
                *payload = (void *)NULL;
                return PERIF_MSG_NONE;
            } else if (!is_m1_within_limits_hist() && !is_aware_m1) {
                *payload = (void *)NULL;
                return PERIF_MSG_NONE;
            } else if (!is_m2_within_limits_hist() && !is_aware_m2) {
                *payload = (void *)NULL;
                return PERIF_MSG_NONE;
            } else if (!is_m3_within_limits_hist() && !is_aware_m3) {
                *payload = (void *)NULL;
                return PERIF_MSG_NONE;
            } else if (!is_m4_within_limits_hist() && !is_aware_m4) {
                *payload = (void *)NULL;
                return PERIF_MSG_NONE;
            } else {
                *payload = (void *)false;
                return PERIF_MSG_NOTIFY_ALARME_STATE;
            }
        }

        /*
        Se o alarme estiver desligado, ele deverá ligar quando:
        - Algum sensor estiver fora da faixa apropriada e is_aware false
        */
    } else {
        if ((!is_entr_within_limits_hist() && !is_aware_entr) ||
            (!is_m1_within_limits_hist() && !is_aware_m1) ||
            (!is_m2_within_limits_hist() && !is_aware_m2) ||
            (!is_m3_within_limits_hist() && !is_aware_m3) ||
            (!is_m4_within_limits_hist() && !is_aware_m4)) {
            *payload = (void *)true;
            return PERIF_MSG_NOTIFY_ALARME_STATE;
        }
    }

    *payload = NULL;
    return PERIF_MSG_NONE;
}

// pegar variaveis no storage, e apartir delas decidir se o alarme deve ser ligada ou desligado
PerifMessageType_t get_perif_queimador_action(void **payload) {
    bool queimador_state = storage_get_queimador_state();
    bool queimador_mode = storage_get_queimador_mode();

    /*
        Se o queimador estiver ligado, ele deverá desligar quando:
        - Estiver em queimador_mode true
        - Algum sensor ultrapassar a máxima dele
    */
   ESP_LOGE("OIEOEIE", "state: %d, mode: %d", queimador_state, queimador_mode);
    if (queimador_state) {
        /*
            Se estiver ligado e em modo lenha, desligar
        */
        if (queimador_mode) {
            *payload = (void *)false;
            return PERIF_MSG_NOTIFY_QUEIMADOR_STATE;
        } else {
            /*
                Se ligado e em modo palha, e algum sensor ultrapassou o limite, desligar
            */
            if (is_entr_higher() ||
                is_m1_higher() ||
                is_m2_higher() ||
                is_m3_higher() ||
                is_m4_higher()) {
                *payload = (void *)false;
                return PERIF_MSG_NOTIFY_QUEIMADOR_STATE;
            }
        }

        /*
        Se o queimador estiver desligado, ele deverá ligar quando:
        - Todos os sensores estiverem abaixo da máxima e estiver em modo palha
        */
    } else {
        /*
            Se estiver desligado, só estarei interessado em ligar denovo quando estiver em modo palha
        */
        if (!queimador_mode) {
            if (!is_entr_higher() && !is_m1_higher() && !is_m2_higher() && !is_m3_higher() && !is_m4_higher()) {
                *payload = (void *)true;
                return PERIF_MSG_NOTIFY_QUEIMADOR_STATE;
            }
        }
    }

    *payload = NULL;
    return PERIF_MSG_NONE;
}

PerifMessageType_t get_perif_led_entr_q_action(void **payload) {
    bool led_state = storage_get_led_entr_q_state();

    /*
        Se o led quente estiver ligado, ele deverá desligar quando:
        - Entr estiver abaixo do máximo
    */
    if (led_state) {
        if (!is_entr_higher()) {
            *payload = (void *)false;
            return PERIF_MSG_NOTIFY_LED_ENTR_Q_STATE;
        }
    } else {
        if (is_entr_higher()) {
            *payload = (void *)true;
            return PERIF_MSG_NOTIFY_LED_ENTR_Q_STATE;
        }
    }

    *payload = NULL;
    return PERIF_MSG_NONE;
}

PerifMessageType_t get_perif_led_entr_f_action(void **payload) {
    bool led_state = storage_get_led_entr_f_state();

    /*
        Se o led frio estiver ligado, ele deverá desligar quando:
        - Entr estiver abaixo do máximo
    */
    if (led_state) {
        if (!is_entr_lower()) {
            *payload = (void *)false;
            return PERIF_MSG_NOTIFY_LED_ENTR_F_STATE;
        }
    } else {
        if (is_entr_lower()) {
            *payload = (void *)true;
            return PERIF_MSG_NOTIFY_LED_ENTR_F_STATE;
        }
    }

    *payload = NULL;
    return PERIF_MSG_NONE;
}

PerifMessageType_t get_perif_led_mass_q_action(void **payload) {
    bool led_state = storage_get_led_mass_q_state();

    /*
        Se o led massa quente estiver ligado, ele deverá desligar quando:
        - Todas as massas estiverem abaixo do máximo

        Ligará quando:
        - Alguma massa estiver acima de seu máximo
    */
    if (led_state) {
        if (!is_m1_higher() && !is_m2_higher() && !is_m3_higher() && !is_m4_higher()) {
            *payload = (void *)false;
            return PERIF_MSG_NOTIFY_LED_MASS_Q_STATE;
        }
    } else {
        if (is_m1_higher() || is_m2_higher() || is_m3_higher() || is_m4_higher()) {
            *payload = (void *)true;
            return PERIF_MSG_NOTIFY_LED_ENTR_Q_STATE;
        }
    }

    *payload = NULL;
    return PERIF_MSG_NONE;
}

PerifMessageType_t get_perif_led_mass_f_action(void **payload) {
    bool led_state = storage_get_led_mass_f_state();

    /*
        Se o led massa frio estiver ligado, ele deverá desligar quando:
        - Todas as massas estiverem acima do mínimo
        - Ou quando alguma massa estiver acima do máximo

        Ligará quando:

        - Se não tiver nenhuma massa acima de seu máximo, e alguma massa estiver abaixo de seu máximo
    */

    if (led_state) {
        if (!is_m1_lower() && !is_m2_lower() && !is_m3_lower() && !is_m4_lower()) {
            *payload = (void *)false;
            return PERIF_MSG_NOTIFY_LED_MASS_F_STATE;
        } else {
            if (is_m1_higher() || is_m2_higher() || is_m3_higher() || is_m4_higher()) {
                *payload = (void *)false;
                return PERIF_MSG_NOTIFY_LED_MASS_F_STATE;
            }
        }
    } else {
        if (!is_m1_higher() && !is_m2_higher() && !is_m3_higher() && !is_m4_higher()) {
            if (is_m1_lower() || is_m2_lower() || is_m3_lower() || is_m4_lower()) {
                *payload = (void *)true;
                return PERIF_MSG_NOTIFY_LED_MASS_F_STATE;
            }
        }
    }

    *payload = NULL;
    return PERIF_MSG_NONE;
}

PerifMessageType_t get_perif_connection_action(void **payload) {
    bool led_state = storage_get_led_connection_state();

    bool is_m1_connected = storage_get_conexao_m1();
    bool is_m2_connected = storage_get_conexao_m2();
    bool is_m3_connected = storage_get_conexao_m3();
    bool is_m4_connected = storage_get_conexao_m4();

    /*
        Se o led conexao estiver ligado, ele deverá desligar quando:
        - Qualquer massa desconectar

        Ligará quando:
        - Todas as massas conectarem
    */

    if (led_state) {
        if (!is_m1_connected || !is_m2_connected || !is_m3_connected || !is_m4_connected) {
            *payload = (void *)false;
            return PERIF_MSG_NOTIFY_LED_CONNECTION_STATE;
        }
    } else {
        if (is_m1_connected && is_m2_connected && is_m3_connected && is_m4_connected) {
            *payload = (void *)true;
            return PERIF_MSG_NOTIFY_LED_CONNECTION_STATE;
        }
    }

    *payload = NULL;
    return PERIF_MSG_NONE;
}

/*
    Se algum alarme estiver ON, mandar a página dele
*/

IHMMessageType_t get_ihm_entr_action(void **payload) {
    bool is_aware_entr = storage_get_is_aware_entr();

    if (!is_entr_within_limits_hist()) {
        if (is_entr_lower_hist() && !is_aware_entr) {
            ESP_LOGE("OIEOIE", "Entrada Lower e não aware!");
            *payload = (void *)-1;
            return IHM_MSG_NOTIFY_ENTR_STATE;
        } else if (is_entr_higher_hist() && !is_aware_entr) {
            ESP_LOGE("OIEOIE", "Entrada HIGHER e não aware!");
            *payload = (void *)1;
            return IHM_MSG_NOTIFY_ENTR_STATE;
        } else {
            ESP_LOGE("OIEOIE", "Nenhum dos dois, min%u - %u - %umax = is_aware = %d", storage_get_min_entr(), storage_get_sensor_entr(), storage_get_max_entr(), storage_get_is_aware_entr());
            *payload = (void *)0;
            return IHM_MSG_NOTIFY_ENTR_STATE;
        }
    }
    *payload = NULL;
    return IHM_MSG_NONE;
}

IHMMessageType_t get_ihm_m1_action(void **payload) {
    bool is_aware_m1 = storage_get_is_aware_m1();
    if (!is_m1_within_limits_hist() && !is_aware_m1) {
        if (is_m1_lower_hist()) {
            *payload = (void *)-1;
            return IHM_MSG_NOTIFY_M1_STATE;
        } else if (is_m1_higher_hist()) {
            *payload = (void *)1;
            return IHM_MSG_NOTIFY_M1_STATE;
        } else {
            *payload = (void *)0;
            return IHM_MSG_NOTIFY_M1_STATE;
        }
    }

    *payload = NULL;
    return IHM_MSG_NONE;
}

IHMMessageType_t get_ihm_m2_action(void **payload) {
    bool is_aware_m2 = storage_get_is_aware_m2();
    if (!is_m2_within_limits_hist() && !is_aware_m2) {
        if (is_m2_lower_hist()) {
            *payload = (void *)-1;
            return IHM_MSG_NOTIFY_M2_STATE;
        } else if (is_m2_higher_hist()) {
            *payload = (void *)1;
            return IHM_MSG_NOTIFY_M2_STATE;
        } else {
            *payload = (void *)0;
            return IHM_MSG_NOTIFY_M2_STATE;
        }
    }

    *payload = NULL;
    return IHM_MSG_NONE;
}

IHMMessageType_t get_ihm_m3_action(void **payload) {
    bool is_aware_m3 = storage_get_is_aware_m3();
    if (!is_m3_within_limits_hist() && !is_aware_m3) {
        if (is_m3_lower_hist()) {
            *payload = (void *)-1;
            return IHM_MSG_NOTIFY_M3_STATE;
        } else if (is_m3_higher_hist()) {
            *payload = (void *)1;
            return IHM_MSG_NOTIFY_M3_STATE;
        } else {
            *payload = (void *)0;
            return IHM_MSG_NOTIFY_M3_STATE;
        }
    }

    *payload = NULL;
    return IHM_MSG_NONE;
}

IHMMessageType_t get_ihm_m4_action(void **payload) {
    bool is_aware_m4 = storage_get_is_aware_m4();
    if (!is_m4_within_limits_hist() && !is_aware_m4) {
        if (is_m4_lower_hist()) {
            *payload = (void *)-1;
            return IHM_MSG_NOTIFY_M4_STATE;
        } else if (is_m4_higher_hist()) {
            *payload = (void *)1;
            return IHM_MSG_NOTIFY_M4_STATE;
        } else {
            *payload = (void *)0;
            return IHM_MSG_NOTIFY_M4_STATE;
        }
    }

    *payload = NULL;
    return IHM_MSG_NONE;
}