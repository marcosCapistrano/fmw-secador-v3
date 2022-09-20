#include "storage.h"

#include <stdio.h>
#include <time.h>

#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "nvs_flash.h"
#include "i2cdev.h"
#include "ds3231.h"

#define NAMESPACE "config"
#define LOTE_NUMBER_KEY "lote_number"
#define LOTE_CONCLUDED_KEY "lote_concluded"

#define QUEIMADOR_MODE_KEY "queim_mode"
#define QUEIMADOR_STATE_KEY "queim_state"

#define SENSOR_ENTR_KEY "sens_entr"
#define SENSOR_M1_KEY "sens_m1"
#define SENSOR_M2_KEY "sens_m2"
#define SENSOR_M3_KEY "sens_m3"
#define SENSOR_M4_KEY "sens_m4"

#define ALARME_ENTR_KEY "alarm_entr"
#define ALARME_M1_KEY "alarm_m1"
#define ALARME_M2_KEY "alarm_m2"
#define ALARME_M3_KEY "alarm_m3"
#define ALARME_M4_KEY "alarm_m4"

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

#define CONEXAO_M1_KEY "m1_connect"
#define CONEXAO_M2_KEY "m2_connect"
#define CONEXAO_M3_KEY "m3_connect"
#define CONEXAO_M4_KEY "m4_connect"

static const char *TAG = "STORAGE";

static nvs_handle_t my_nvs_handle;

static uint8_t lote_number = 0;
static bool lote_concluded = true;   // FALSE = Começado, não finalizado, TRUE = Finalizado
static bool queimador_mode = false;  // FALSE = Palha, TRUE = Lenha
static bool queimador_state = false;

static uint8_t sensor_entr = 0;
static uint8_t sensor_m1 = 0;
static uint8_t sensor_m2 = 0;
static uint8_t sensor_m3 = 0;
static uint8_t sensor_m4 = 0;

static bool alarm_entr = false;
static bool alarm_m1 = false;
static bool alarm_m2 = false;
static bool alarm_m3 = false;
static bool alarm_m4 = false;

static uint8_t limit_min_entr = 0;
static uint8_t limit_max_entr = 100;
static uint8_t limit_min_m1 = 0;
static uint8_t limit_max_m1 = 100;
static uint8_t limit_min_m2 = 0;
static uint8_t limit_max_m2 = 100;
static uint8_t limit_min_m3 = 0;
static uint8_t limit_max_m3 = 100;
static uint8_t limit_min_m4 = 0;
static uint8_t limit_max_m4 = 100;

static bool conexao_m1 = false;
static bool conexao_m2 = false;
static bool conexao_m3 = false;
static bool conexao_m4 = false;

static i2c_dev_t rtc_dev;

static bool get_bool(const char *key, bool base_value) {
    uint8_t temp_out;

    esp_err_t err = nvs_get_u8(my_nvs_handle, key, &temp_out);
    if (err != ESP_OK) {
        nvs_set_u8(my_nvs_handle, key, base_value);
        return base_value;
    }

    if (temp_out == 0) {
        return false;
    } else {
        return true;
    }
}
static void set_bool(const char *key, bool new_value) {
    if (new_value) {
        ESP_ERROR_CHECK(nvs_set_u8(my_nvs_handle, key, 1));
    } else {
        ESP_ERROR_CHECK(nvs_set_u8(my_nvs_handle, key, 0));
    }

    nvs_commit(my_nvs_handle);
}
static uint8_t get_u8(const char *key, uint8_t base_value) {
    uint8_t temp_out;

    esp_err_t err = nvs_get_u8(my_nvs_handle, key, &temp_out);
    if (err != ESP_OK) {
        nvs_set_u8(my_nvs_handle, key, base_value);
        return base_value;
    }

    return temp_out;
}
static void set_u8(const char *key, uint8_t new_value) {
    ESP_ERROR_CHECK(nvs_set_u8(my_nvs_handle, key, new_value));

    nvs_commit(my_nvs_handle);
}

uint8_t storage_get_lote_number() {
    return lote_number;
}
void storage_set_lote_number(uint8_t new_value) {
    if (lote_number != new_value) {
        set_u8(LOTE_NUMBER_KEY, new_value);
        lote_number = new_value;
    }
}

bool storage_get_lote_concluded() {
    return lote_concluded;
}
void storage_set_lote_concluded(bool new_value) {
    if (lote_concluded != new_value) {
        set_bool(LOTE_CONCLUDED_KEY, new_value);
        lote_concluded = new_value;
    }
}

bool storage_get_queimador_mode() {
    return queimador_mode;
}
void storage_set_queimador_mode(bool new_value) {
    if (queimador_mode != new_value) {
        set_bool(QUEIMADOR_MODE_KEY, new_value);
        queimador_mode = new_value;
    }
}

bool storage_get_queimador_state() {
    return queimador_state;
}
void storage_set_queimador_state(bool new_value) {
    if (queimador_state != new_value) {
        set_bool(QUEIMADOR_STATE_KEY, new_value);
        queimador_state = new_value;
    }
}

uint8_t storage_get_sensor_entr() {
    return sensor_entr;
}
void storage_set_sensor_entr(uint8_t new_value) {
    if (sensor_entr != new_value) {
        set_u8(SENSOR_ENTR_KEY, new_value);
        sensor_entr = new_value;
    }
}

uint8_t storage_get_sensor_m1() {
    return sensor_m1;
}
void storage_set_sensor_m1(uint8_t new_value) {
    if (sensor_m1 != new_value) {
        set_u8(SENSOR_M1_KEY, new_value);
        sensor_m1 = new_value;
    }
}

uint8_t storage_get_sensor_m2() {
    return sensor_m2;
}
void storage_set_sensor_m2(uint8_t new_value) {
    if (sensor_m2 != new_value) {
        set_u8(SENSOR_M2_KEY, new_value);
        sensor_m2 = new_value;
    }
}

uint8_t storage_get_sensor_m3() {
    return sensor_m3;
}
void storage_set_sensor_m3(uint8_t new_value) {
    if (sensor_m3 != new_value) {
        set_u8(SENSOR_M3_KEY, new_value);
        sensor_m3 = new_value;
    }
}

uint8_t storage_get_sensor_m4() {
    return sensor_m4;
}
void storage_set_sensor_m4(uint8_t new_value) {
    if (sensor_m4 != new_value) {
        set_u8(SENSOR_M4_KEY, new_value);
        sensor_m4 = new_value;
    }
}

bool storage_get_alarme_entr() {
    return alarm_entr;
}
void storage_set_alarme_entr(bool new_value) {
    if (alarm_entr != new_value) {
        set_bool(ALARME_ENTR_KEY, new_value);
        alarm_entr = new_value;
    }
}

bool storage_get_alarme_m1() {
    return alarm_m1;
}
void storage_set_alarme_m1(bool new_value) {
    if (alarm_m1 != new_value) {
        set_bool(ALARME_M1_KEY, new_value);
        alarm_m1 = new_value;
    }
}

bool storage_get_alarme_m2() {
    return alarm_m2;
}
void storage_set_alarme_m2(bool new_value) {
    if (alarm_m2 != new_value) {
        set_bool(ALARME_M2_KEY, new_value);
        alarm_m2 = new_value;
    }
}

bool storage_get_alarme_m3() {
    return alarm_m3;
}
void storage_set_alarme_m3(bool new_value) {
    if (alarm_m3 != new_value) {
        set_bool(ALARME_M3_KEY, new_value);
        alarm_m3 = new_value;
    }
}

bool storage_get_alarme_m4() {
    return alarm_m4;
}
void storage_set_alarme_m4(bool new_value) {
    if (alarm_m4 != new_value) {
        set_bool(ALARME_M4_KEY, new_value);
        alarm_m4 = new_value;
    }
}

uint8_t storage_get_min_entr() {
    return limit_min_entr;
}
void storage_set_min_entr(uint8_t new_value) {
    if (limit_min_entr != new_value) {
        set_u8(ENTR_MIN_KEY, new_value);
        limit_min_entr = new_value;
    }
}

uint8_t storage_get_max_entr() {
    return limit_max_entr;
}
void storage_set_max_entr(uint8_t new_value) {
    if (limit_max_entr != new_value) {
        set_u8(ENTR_MAX_KEY, new_value);
        limit_max_entr = new_value;
    }
}

uint8_t storage_get_min_m1() {
    return limit_min_m1;
}
void storage_set_min_m1(uint8_t new_value) {
    if (limit_min_m1 != new_value) {
        set_u8(M1_MIN_KEY, new_value);
        limit_min_m1 = new_value;
    }
}

uint8_t storage_get_max_m1() {
    return limit_max_m1;
}
void storage_set_max_m1(uint8_t new_value) {
    if (limit_max_m1 != new_value) {
        set_u8(M1_MAX_KEY, new_value);
        limit_max_m1 = new_value;
    }
}

uint8_t storage_get_min_m2() {
    return limit_min_m2;
}
void storage_set_min_m2(uint8_t new_value) {
    if (limit_min_m2 != new_value) {
        set_u8(M2_MIN_KEY, new_value);
        limit_min_m2 = new_value;
    }
}

uint8_t storage_get_max_m2() {
    return limit_max_m2;
}
void storage_set_max_m2(uint8_t new_value) {
    if (limit_max_m2 != new_value) {
        set_u8(M2_MAX_KEY, new_value);
        limit_max_m2 = new_value;
    }
}

uint8_t storage_get_min_m3() {
    return limit_min_m3;
}
void storage_set_min_m3(uint8_t new_value) {
    if (limit_min_m3 != new_value) {
        set_u8(M3_MIN_KEY, new_value);
        limit_min_m3 = new_value;
    }
}

uint8_t storage_get_max_m3() {
    return limit_max_m3;
}
void storage_set_max_m3(uint8_t new_value) {
    if (limit_max_m3 != new_value) {
        set_u8(M3_MAX_KEY, new_value);
        limit_max_m3 = new_value;
    }
}

uint8_t storage_get_min_m4() {
    return limit_min_m4;
}
void storage_set_min_m4(uint8_t new_value) {
    if (limit_min_m4 != new_value) {
        set_u8(M4_MIN_KEY, new_value);
        limit_min_m4 = new_value;
    }
}

uint8_t storage_get_max_m4() {
    return limit_max_m4;
}
void storage_set_max_m4(uint8_t new_value) {
    if (limit_max_m4 != new_value) {
        set_u8(M4_MAX_KEY, new_value);
        limit_max_m4 = new_value;
    }
}

bool storage_get_conexao_m1() {
    return conexao_m1;
}
void storage_set_conexao_m1(bool new_value) {
    if (conexao_m1 != new_value) {
        set_bool(CONEXAO_M1_KEY, new_value);
        conexao_m1 = new_value;
    }
}

bool storage_get_conexao_m2() {
    return conexao_m2;
}
void storage_set_conexao_m2(bool new_value) {
    if (conexao_m2 != new_value) {
        set_bool(CONEXAO_M2_KEY, new_value);
        conexao_m2 = new_value;
    }
}

bool storage_get_conexao_m3() {
    return conexao_m3;
}
void storage_set_conexao_m3(bool new_value) {
    if (conexao_m3 != new_value) {
        set_bool(CONEXAO_M3_KEY, new_value);
        conexao_m3 = new_value;
    }
}

bool storage_get_conexao_m4() {
    return conexao_m4;
}
void storage_set_conexao_m4(bool new_value) {
    if (conexao_m4 != new_value) {
        set_bool(CONEXAO_M4_KEY, new_value);
        conexao_m4 = new_value;
    }
}

void storage_get_all_lotes() {
}

static void initialize_cache() {
    lote_number = get_u8(LOTE_NUMBER_KEY, 0);
    lote_concluded = get_bool(LOTE_CONCLUDED_KEY, true);

    queimador_mode = get_bool(QUEIMADOR_MODE_KEY, false);
    queimador_state = get_bool(QUEIMADOR_STATE_KEY, false);

    sensor_entr = get_u8(SENSOR_ENTR_KEY, 0);
    sensor_m1 = get_u8(SENSOR_M1_KEY, 0);
    sensor_m2 = get_u8(SENSOR_M2_KEY, 0);
    sensor_m3 = get_u8(SENSOR_M3_KEY, 0);
    sensor_m4 = get_u8(SENSOR_M4_KEY, 0);

    alarm_entr = get_bool(ALARME_ENTR_KEY, false);
    alarm_m1 = get_bool(ALARME_M1_KEY, false);
    alarm_m2 = get_bool(ALARME_M2_KEY, false);
    alarm_m3 = get_bool(ALARME_M3_KEY, false);
    alarm_m4 = get_bool(ALARME_M4_KEY, false);

    limit_min_entr = get_u8(ENTR_MIN_KEY, 0);
    limit_max_entr = get_u8(ENTR_MAX_KEY, 100);
    limit_min_m1 = get_u8(M1_MIN_KEY, 0);
    limit_max_m1 = get_u8(M1_MAX_KEY, 100);
    limit_min_m2 = get_u8(M2_MIN_KEY, 0);
    limit_max_m2 = get_u8(M2_MAX_KEY, 100);
    limit_min_m3 = get_u8(M3_MIN_KEY, 0);
    limit_max_m3 = get_u8(M3_MAX_KEY, 100);
    limit_min_m4 = get_u8(M4_MIN_KEY, 0);
    limit_max_m4 = get_u8(M4_MAX_KEY, 100);

    ESP_LOGE(TAG, "Limits M1 %d - %d", limit_min_m1, limit_max_m1);
    ESP_LOGE(TAG, "Limits M2 %d - %d", limit_min_m2, limit_max_m2);

    conexao_m1 = get_bool(CONEXAO_M1_KEY, false);
    conexao_m2 = get_bool(CONEXAO_M2_KEY, false);
    conexao_m3 = get_bool(CONEXAO_M3_KEY, false);
    conexao_m4 = get_bool(CONEXAO_M4_KEY, false);
}

static void initialize_fs() {
    esp_vfs_spiffs_conf_t web_conf = {
        .base_path = "/website",
        .partition_label = "website",
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_vfs_spiffs_conf_t storage_conf = {
        .base_path = "/storage",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true};

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&web_conf));
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&storage_conf));

    size_t total = 0, used = 0;
    esp_err_t ret = esp_spiffs_info("website", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    ret = esp_spiffs_info("storage", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}

static bool file_exists(const char *path) {
    if (access(path, F_OK) == 0)
        return true;

    return false;
}

static time_t get_time_epoch() {
    struct tm rtcinfo;

    if (ds3231_get_time(&rtc_dev, &rtcinfo) != ESP_OK) {
        ESP_LOGE(TAG, "Could not get time.");
    }

    time_t time = mktime(&rtcinfo);
    return time;
}

void storage_add_record_init() {
    const char path[100] = {0};
    time_t time_now = get_time_epoch();

    sprintf(path, "/storage/%d", lote_number);

    FILE *f = fopen(path, "w");
    fprintf(f, "%ld,INIT,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", time_now, queimador_mode, limit_min_entr, limit_max_entr, limit_min_m1, limit_max_m1, limit_min_m2, limit_max_m2, limit_min_m3, limit_max_m3, limit_min_m4, limit_max_m4);

    fclose(f);
}

void storage_add_record_device_on() {
    const char path[100] = {0};
    time_t time_now = get_time_epoch();

    sprintf(path, "/storage/%d", lote_number);

    FILE *f = fopen(path, "a");
    fprintf(f, "%ld,DEVICE_STATE,%d\n", time_now, true);

    fclose(f);
}

void storage_add_record_finish() {
    const char path[100] = {0};
    time_t time_now = get_time_epoch();

    sprintf(path, "/storage/%d", lote_number);

    FILE *f = fopen(path, "a");
    fprintf(f, "%ld,FINISHED,%d\n", time_now, true);
    fclose(f);

    storage_set_lote_concluded(true);
}

static const char *event_type_to_string(StorageEventType_t type) {
    switch(type) {
        case EVENT_SENSOR_ENTR:
            return "SENSOR_ENTR";
        break;

        default:
        break;
    }

    return "NULL";
}

void storage_add_record_generic(StorageEventType_t type, void *payload) {
    const char path[100] = {0};
    time_t time_now = get_time_epoch();
    sprintf(path, "/storage/%d", lote_number);

    FILE *f = fopen(path, "a");

    const char *type_str = event_type_to_string(type);
    fprintf(f, "%ld,%s,%d\n", time_now, type_str, (int) payload);

    fclose(f);
}

void storage_start_new_lote() {
    uint8_t new_lote_number = lote_number + 1;

    storage_set_lote_number(new_lote_number);
    storage_set_lote_concluded(false);
}

void storage_init(void) {
    ESP_LOGI(TAG, "Inicializando Armazenamento...");
    esp_err_t err;

    // NVS para Configurações persistentes e persistencia de alguns outros valores
    err = nvs_open("config", NVS_READWRITE, &my_nvs_handle);
    ESP_ERROR_CHECK(err);

    if (ds3231_init_desc(&rtc_dev, I2C_NUM_0, CONFIG_RTC_SDA_GPIO, CONFIG_RTC_SCL_GPIO) != ESP_OK) {
        ESP_LOGE(TAG, "Could not init device descriptor.");
    } else {
        ESP_LOGI(TAG, "Found RTC!");

        struct tm rtcinfo;

        if (ds3231_get_time(&rtc_dev, &rtcinfo) != ESP_OK) {
            ESP_LOGE(TAG, "Could not get time.");
        }

        ESP_LOGI(TAG, "Current date:");
        ESP_LOGI(TAG, "%04d-%02d-%02d %02d:%02d:%02d:000Z", rtcinfo.tm_year, rtcinfo.tm_mon + 1, rtcinfo.tm_mday, rtcinfo.tm_hour, rtcinfo.tm_min, rtcinfo.tm_sec);
    }

    initialize_cache();
    initialize_fs();
}
