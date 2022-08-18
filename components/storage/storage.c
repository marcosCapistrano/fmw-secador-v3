#include "storage.h"

#include <stdbool.h>

#include "common.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"

#define NAMESPACE "config"

#define PL_KEY "palha_lenha"

#define ENTR_MIN_KEY "entr_min"
#define ENTR_MAX_KEY "entr_max"

#define M1_MIN_KEY "m1_min"
#define M1_MAX_KEY "m1_max"

#define M2_MIN_KEY "m2_min"
#define M2_MAX_KEY "m2_max"

#define M3_MIN_KEY "m3_min"
#define M3_MAX_KEY "m3_max"

#define M4_MIN_KEY "m4_min"
#define M4_MAX_KEY "m4_max"

static bool queimadorMode;  // Palha ou Lenha (0 ou  1)
static bool isQueimadorOn;
static bool isAlarmeOn;

static bool isM1Connected;
static bool isM2Connected;
static bool isM3Connected;
static bool isM4Connected;

static bool isAwareEntr;
static bool isAwareM1;
static bool isAwareM2;
static bool isAwareM3;
static bool isAwareM4;

static uint8_t tempEntr;
static uint8_t tempM1;
static uint8_t tempM2;
static uint8_t tempM3;
static uint8_t tempM4;

static uint8_t minTempEntr;
static uint8_t maxTempEntr;

static uint8_t minTempM1;
static uint8_t maxTempM1;

static uint8_t minTempM2;
static uint8_t maxTempM2;

static uint8_t minTempM3;
static uint8_t maxTempM3;

static uint8_t minTempM4;
static uint8_t maxTempM4;

static nvs_handle_t nvsHandle;

static bool get_bool(const char* key, uint8_t base_value);
static uint8_t get_u8(const char* key, uint8_t base_value);

void storage_init() {
    // Ler valores nvs
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(nvs_open(NAMESPACE, NVS_READWRITE, &nvsHandle));

    queimadorMode = get_bool(PL_KEY, false);
    isQueimadorOn = false;
    isAlarmeOn = false;

    isM1Connected = false;
    isM2Connected = false;
    isM3Connected = false;
    isM4Connected = false;

    isAwareEntr = false;
    isAwareM1 = false;
    isAwareM2 = false;
    isAwareM3 = false;
    isAwareM4 = false;

    tempEntr = 0;
    tempM1 = 0;
    tempM2 = 0;
    tempM3 = 0;
    tempM4 = 0;

    minTempEntr = get_u8(ENTR_MIN_KEY, 0);
    maxTempEntr = get_u8(ENTR_MAX_KEY, 100);

    minTempM1 = get_u8(M1_MIN_KEY, 0);
    maxTempM1 = get_u8(M1_MAX_KEY, 100);

    minTempM2 = get_u8(M2_MIN_KEY, 0);
    maxTempM2 = get_u8(M2_MAX_KEY, 100);

    minTempM3 = get_u8(M3_MIN_KEY, 0);
    maxTempM3 = get_u8(M3_MAX_KEY, 100);

    minTempM4 = get_u8(M4_MIN_KEY, 0);
    maxTempM4 = get_u8(M4_MAX_KEY, 100);
}

static bool get_bool(const char* key, uint8_t base_value) {
    uint8_t temp_out;

    esp_err_t err = nvs_get_u8(nvsHandle, key, &temp_out);
    if (err != ESP_OK) {
        nvs_set_u8(nvsHandle, key, base_value);

        if (base_value == 0) {
            return false;
        } else {
            return true;
        }
    } else {
        if (temp_out == 0) {
            return false;
        } else {
            return true;
        }
    }
}

static uint8_t get_u8(const char* key, uint8_t base_value) {
    uint8_t temp_out;

    esp_err_t err = nvs_get_u8(nvsHandle, key, &temp_out);
    if (err != ESP_OK) {
        nvs_set_u8(nvsHandle, key, base_value);
        return base_value;
    }

    return temp_out;
}

static void set_bool(const char* key, bool new_value) {
    if (new_value) {
        ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, key, 1));
    } else {
        ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, key, 0));
    }
}

static void set_u8(const char* key, uint8_t new_value) {
    ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, key, new_value));
}

bool get_queimador_mode() {
    return queimadorMode;
}

bool get_queimador_state() {
    return isQueimadorOn;
}

bool get_alarme_state() {
    return isAlarmeOn;
}

bool get_m1_connected() {
    return isM1Connected;
}

bool get_m2_connected() {
    return isM1Connected;
}

bool get_m3_connected() {
    return isM1Connected;
}

bool get_m4_connected() {
    return isM1Connected;
}

bool get_entr_aware() {
    return isAwareEntr;
}

bool get_m1_aware() {
    return isAwareEntr;
}

bool get_m2_aware() {
    return isAwareEntr;
}

bool get_m3_aware() {
    return isAwareEntr;
}

bool get_m4_aware() {
    return isAwareEntr;
}

uint8_t get_temp_entr() {
    return tempEntr;
}

uint8_t get_temp_m1() {
    return tempM1;
}

uint8_t get_temp_m2() {
    return tempM2;
}

uint8_t get_temp_m3() {
    return tempM3;
}

uint8_t get_temp_m4() {
    return tempM4;
}

uint8_t get_min_temp_entr() {
    return minTempEntr;
}

uint8_t get_max_temp_entr() {
    return maxTempEntr;
}

uint8_t get_min_temp_m1() {
    return minTempM1;
}

uint8_t get_max_temp_m1() {
    return maxTempM1;
}

uint8_t get_min_temp_m2() {
    return minTempM2;
}

uint8_t get_max_temp_m2() {
    return maxTempM2;
}

uint8_t get_min_temp_m3() {
    return minTempM3;
}

uint8_t get_max_temp_m3() {
    return maxTempM3;
}

uint8_t get_min_temp_m4() {
    return minTempM4;
}

uint8_t get_max_temp_m4() {
    return maxTempM4;
}

void set_queimador_mode(bool new_value) {
    if (get_queimador_mode() != new_value) {
        set_bool(PL_KEY, new_value);
        queimadorMode = new_value;
    }
}

void set_queimador_state(bool new_value) {
    if (get_queimador_state() != new_value) {
        isQueimadorOn = new_value;
    }
}

void set_alarme_state(bool new_value) {
    if (get_alarme_state() != new_value) {
        isAlarmeOn = new_value;
    }
}

void set_m1_connected(bool new_value) {
    if (get_m1_connected() != new_value) {
        isM1Connected = new_value;
    }
}

void set_m2_connected(bool new_value) {
    if (get_m2_connected() != new_value) {
        isM2Connected = new_value;
    }
}

void set_m3_connected(bool new_value) {
    if (get_m3_connected() != new_value) {
        isM3Connected = new_value;
    }
}

void set_m4_connected(bool new_value) {
    if (get_m4_connected() != new_value) {
        isM4Connected = new_value;
    }
}

void set_entr_aware(bool new_value) {
    if (get_entr_aware() != new_value) {
        isAwareEntr = new_value;
    }
}

void set_m1_aware(bool new_value) {
    if (get_m1_aware() != new_value) {
        isAwareM1 = new_value;
    }
}

void set_m2_aware(bool new_value) {
    if (get_m2_aware() != new_value) {
        isAwareM2 = new_value;
    }
}

void set_m3_aware(bool new_value) {
    if (get_m3_aware() != new_value) {
        isAwareM3 = new_value;
    }
}

void set_m4_aware(bool new_value) {
    if (get_m4_aware() != new_value) {
        isAwareM4 = new_value;
    }
}

void set_temp_entr(uint8_t new_value) {
    if(get_temp_entr() != new_value) {
        tempEntr = new_value;
    }
}

void set_temp_m1(uint8_t new_value) {
    if(get_temp_m1() != new_value) {
        tempM1 = new_value;
    }
}

void set_temp_m2(uint8_t new_value) {
    if(get_temp_m2() != new_value) {
        tempM2 = new_value;
    }
}

void set_temp_m3(uint8_t new_value) {
    if(get_temp_m3() != new_value) {
        tempM3 = new_value;
    }
}

void set_temp_m4(uint8_t new_value) {
    if(get_temp_m4() != new_value) {
        tempM4 = new_value;
    }
}

void set_min_temp_entr(uint8_t new_value) {
    if(get_min_temp_entr() != new_value) {
        set_u8(ENTR_MIN_KEY, new_value);
        minTempEntr = new_value;
    }
}

void set_max_temp_entr(uint8_t new_value) {
    if(get_max_temp_entr() != new_value) {
        set_u8(ENTR_MAX_KEY, new_value);
        maxTempEntr = new_value;
    }
}

void set_min_temp_m1(uint8_t new_value) {
    if(get_min_temp_m1() != new_value) {
        set_u8(M1_MIN_KEY, new_value);
        minTempM1 = new_value;
    }
}

void set_max_temp_m1(uint8_t new_value) {
    if(get_max_temp_m1() != new_value) {
        set_u8(M1_MAX_KEY, new_value);
        maxTempM1 = new_value;
    }
}

void set_min_temp_m2(uint8_t new_value) {
    if(get_min_temp_m2() != new_value) {
        set_u8(M2_MIN_KEY, new_value);
        minTempM2 = new_value;
    }
}

void set_max_temp_m2(uint8_t new_value) {
    if(get_max_temp_m2() != new_value) {
        set_u8(M2_MAX_KEY, new_value);
        maxTempM2 = new_value;
    }
}

void set_min_temp_m3(uint8_t new_value) {
    if(get_min_temp_m3() != new_value) {
        set_u8(M3_MIN_KEY, new_value);
        minTempM3 = new_value;
    }
}

void set_max_temp_m3(uint8_t new_value) {
    if(get_max_temp_m3() != new_value) {
        set_u8(M3_MAX_KEY, new_value);
        maxTempM3 = new_value;
    }
}

void set_min_temp_m4(uint8_t new_value) {
    if(get_min_temp_m4() != new_value) {
        set_u8(M4_MIN_KEY, new_value);
        minTempM4 = new_value;
    }
}

void set_max_temp_m4(uint8_t new_value) {
    if(get_max_temp_m4() != new_value) {
        set_u8(M4_MAX_KEY, new_value);
        maxTempM4 = new_value;
    }
}