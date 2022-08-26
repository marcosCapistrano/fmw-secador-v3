#include "storage.h"

#include <flashdb.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "STORAGE";

static int lote_number = 0;
static bool lote_finished = true;
static bool queimador_mode = false;  // Palha ou Lenha (0 ou  1)

static int min_temp_entr = 0;
static int max_temp_entr = 100;

static int min_temp_m1 = 0;
static int max_temp_m1 = 100;

static int min_temp_m2 = 0;
static int max_temp_m2 = 100;

static int min_temp_m3 = 0;
static int max_temp_m3 = 100;

static int min_temp_m4 = 0;
static int max_temp_m4 = 100;

static struct fdb_default_kv_node sto_config_table[] = {
    {"lote_number", &lote_number, sizeof(lote_number)},          /* string KV */
    {"lote_finished", &lote_finished, sizeof(lote_finished)},    /* string KV */
    {"queimador_mode", &queimador_mode, sizeof(queimador_mode)}, /* string KV */
    {"min_temp_entr", &min_temp_entr, sizeof(min_temp_entr)},    /* string KV */
    {"max_temp_entr", &max_temp_entr, sizeof(max_temp_entr)},    /* string KV */
    {"min_temp_m1", &min_temp_m1, sizeof(min_temp_m1)},          /* string KV */
    {"max_temp_m1", &max_temp_m1, sizeof(max_temp_m1)},          /* string KV */
    {"min_temp_m2", &min_temp_m2, sizeof(min_temp_m2)},          /* string KV */
    {"max_temp_m2", &max_temp_m2, sizeof(max_temp_m2)},          /* string KV */
    {"min_temp_m3", &min_temp_m3, sizeof(min_temp_m3)},          /* string KV */
    {"max_temp_m3", &max_temp_m3, sizeof(max_temp_m3)},          /* string KV */
    {"min_temp_m4", &min_temp_m4, sizeof(min_temp_m4)},          /* string KV */
    {"max_temp_m4", &max_temp_m4, sizeof(max_temp_m4)},          /* string KV */
};
static struct fdb_kvdb kvdb = {0};

struct fdb_tsdb tsdb = {0};

static SemaphoreHandle_t s_lock = NULL;

static void lock(fdb_db_t db) {
    xSemaphoreTake(s_lock, portMAX_DELAY);
}

static void unlock(fdb_db_t db) {
    xSemaphoreGive(s_lock);
}

typedef enum {
    EVENT_LOTE_FINISHED,
    EVENT_LOTE_STARTED,
    EVENT_LOTE_NUM_CHANGED,
} StorageEventType_t;

typedef struct {
    StorageEventType_t type;
    int value;
} StorageEvent_t;

static bool query_cb(fdb_tsl_t tsl, void *arg);
static bool query_by_time_cb(fdb_tsl_t tsl, void *arg);
static bool set_status_cb(fdb_tsl_t tsl, void *arg);

int counts = 0;

static fdb_time_t get_time(void) {
    /* Using the counts instead of timestamp.
     * Please change this function to return RTC time.
     */
    return ++counts;
}

int storage_get_lote_number() {
    struct fdb_blob blob;
    int sto_lote_num = 0;
    fdb_kv_get_blob(&kvdb, "lote_number", fdb_blob_make(&blob, &sto_lote_num, sizeof(sto_lote_num)));

    return sto_lote_num;
}

void storage_set_lote_number(int new_value) {
    struct fdb_blob blob;
    int sto_lote_num = new_value;
    if (storage_get_lote_number() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "lote_number", fdb_blob_make(&blob, &sto_lote_num, sizeof(sto_lote_num)));
    }
}

bool storage_get_lote_finished() {
    struct fdb_blob blob;
    bool sto_lote_finished = false;
    fdb_kv_get_blob(&kvdb, "lote_finished", fdb_blob_make(&blob, &sto_lote_finished, sizeof(sto_lote_finished)));

    return sto_lote_finished;
}

void storage_set_lote_finished(bool new_value) {
    struct fdb_blob blob;
    bool sto_lote_finished = new_value;
    if (storage_get_lote_finished() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "lote_finished", fdb_blob_make(&blob, &sto_lote_finished, sizeof(sto_lote_finished)));
    }
}

bool storage_get_queimador_mode() {
    struct fdb_blob blob;
    bool sto_queimador_mode = false;
    fdb_kv_get_blob(&kvdb, "queimador_mode", fdb_blob_make(&blob, &sto_queimador_mode, sizeof(sto_queimador_mode)));

    return sto_queimador_mode;
}

void storage_set_queimador_mode(bool new_value) {
    struct fdb_blob blob;
    bool sto_queimador_mode = new_value;
    if (storage_get_queimador_mode() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "queimador_mode", fdb_blob_make(&blob, &sto_queimador_mode, sizeof(sto_queimador_mode)));
    }
}

int storage_get_min_temp_entr() {
    struct fdb_blob blob;
    int sto_min_temp_entr = 0;
    fdb_kv_get_blob(&kvdb, "min_temp_entr", fdb_blob_make(&blob, &sto_min_temp_entr, sizeof(sto_min_temp_entr)));

    return sto_min_temp_entr;
}

void storage_set_min_temp_entr(int new_value) {
    struct fdb_blob blob;
    int sto_min_temp_entr = new_value;
    if (storage_get_min_temp_entr() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "min_temp_entr", fdb_blob_make(&blob, &sto_min_temp_entr, sizeof(sto_min_temp_entr)));
    }
}

int storage_get_max_temp_entr() {
    struct fdb_blob blob;
    int sto_max_temp_entr = 0;
    fdb_kv_get_blob(&kvdb, "max_temp_entr", fdb_blob_make(&blob, &sto_max_temp_entr, sizeof(sto_max_temp_entr)));

    return sto_max_temp_entr;
}

void storage_set_max_temp_entr(int new_value) {
    struct fdb_blob blob;
    int sto_max_temp_entr = new_value;
    if (storage_get_max_temp_entr() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "max_temp_entr", fdb_blob_make(&blob, &sto_max_temp_entr, sizeof(sto_max_temp_entr)));
    }
}

int storage_get_min_temp_m1() {
    struct fdb_blob blob;
    int sto_min_temp_m1 = 0;
    fdb_kv_get_blob(&kvdb, "min_temp_m1", fdb_blob_make(&blob, &sto_min_temp_m1, sizeof(sto_min_temp_m1)));

    return sto_min_temp_m1;
}

void storage_set_min_temp_m1(int new_value) {
    struct fdb_blob blob;
    int sto_min_temp_m1 = new_value;
    if (storage_get_min_temp_m1() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "min_temp_m1", fdb_blob_make(&blob, &sto_min_temp_m1, sizeof(sto_min_temp_m1)));
    }
}

int storage_get_max_temp_m1() {
    struct fdb_blob blob;
    int sto_max_temp_m1 = 0;
    fdb_kv_get_blob(&kvdb, "max_temp_m1", fdb_blob_make(&blob, &sto_max_temp_m1, sizeof(sto_max_temp_m1)));

    return sto_max_temp_m1;
}

void storage_set_max_temp_m1(int new_value) {
    struct fdb_blob blob;
    int sto_max_temp_m1 = new_value;
    if (storage_get_max_temp_m1() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "max_temp_m1", fdb_blob_make(&blob, &sto_max_temp_m1, sizeof(sto_max_temp_m1)));
    }
}

int storage_get_min_temp_m2() {
    struct fdb_blob blob;
    int sto_min_temp_m2 = 0;
    fdb_kv_get_blob(&kvdb, "min_temp_m2", fdb_blob_make(&blob, &sto_min_temp_m2, sizeof(sto_min_temp_m2)));

    return sto_min_temp_m2;
}

void storage_set_min_temp_m2(int new_value) {
    struct fdb_blob blob;
    int sto_min_temp_m2 = new_value;
    if (storage_get_min_temp_m2() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "min_temp_m2", fdb_blob_make(&blob, &sto_min_temp_m2, sizeof(sto_min_temp_m2)));
    }
}

int storage_get_max_temp_m2() {
    struct fdb_blob blob;
    int sto_max_temp_m2 = 0;
    fdb_kv_get_blob(&kvdb, "max_temp_m2", fdb_blob_make(&blob, &sto_max_temp_m2, sizeof(sto_max_temp_m2)));

    return sto_max_temp_m2;
}

void storage_set_max_temp_m2(int new_value) {
    struct fdb_blob blob;
    int sto_max_temp_m2 = new_value;
    if (storage_get_max_temp_m2() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "max_temp_m2", fdb_blob_make(&blob, &sto_max_temp_m2, sizeof(sto_max_temp_m2)));
    }
}

int storage_get_min_temp_m3() {
    struct fdb_blob blob;
    int sto_min_temp_m3 = 0;
    fdb_kv_get_blob(&kvdb, "min_temp_m3", fdb_blob_make(&blob, &sto_min_temp_m3, sizeof(sto_min_temp_m3)));

    return sto_min_temp_m3;
}

void storage_set_min_temp_m3(int new_value) {
    struct fdb_blob blob;
    int sto_min_temp_m3 = new_value;
    if (storage_get_min_temp_m3() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "min_temp_m3", fdb_blob_make(&blob, &sto_min_temp_m3, sizeof(sto_min_temp_m3)));
    }
}

int storage_get_max_temp_m3() {
    struct fdb_blob blob;
    int sto_max_temp_m3 = 0;
    fdb_kv_get_blob(&kvdb, "max_temp_m3", fdb_blob_make(&blob, &sto_max_temp_m3, sizeof(sto_max_temp_m3)));

    return sto_max_temp_m3;
}

void storage_set_max_temp_m3(int new_value) {
    struct fdb_blob blob;
    int sto_max_temp_m3 = new_value;
    if (storage_get_max_temp_m3() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "max_temp_m3", fdb_blob_make(&blob, &sto_max_temp_m3, sizeof(sto_max_temp_m3)));
    }
}

int storage_get_min_temp_m4() {
    struct fdb_blob blob;
    int sto_min_temp_m4 = 0;
    fdb_kv_get_blob(&kvdb, "min_temp_m4", fdb_blob_make(&blob, &sto_min_temp_m4, sizeof(sto_min_temp_m4)));

    return sto_min_temp_m4;
}

void storage_set_min_temp_m4(int new_value) {
    struct fdb_blob blob;
    int sto_min_temp_m4 = new_value;
    if (storage_get_min_temp_m4() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "min_temp_m4", fdb_blob_make(&blob, &sto_min_temp_m4, sizeof(sto_min_temp_m4)));
    }
}

int storage_get_max_temp_m4() {
    struct fdb_blob blob;
    int sto_max_temp_m4 = 0;
    fdb_kv_get_blob(&kvdb, "max_temp_m4", fdb_blob_make(&blob, &sto_max_temp_m4, sizeof(sto_max_temp_m4)));

    return sto_max_temp_m4;
}

void storage_set_max_temp_m4(int new_value) {
    struct fdb_blob blob;
    int sto_max_temp_m4 = new_value;
    if (storage_get_max_temp_m4() != new_value) {
        // set new value on kvdb
        fdb_kv_set_blob(&kvdb, "max_temp_m4", fdb_blob_make(&blob, &sto_max_temp_m4, sizeof(sto_max_temp_m4)));
    }
}

void storage_init(void) {
    ESP_LOGI(TAG, "Inicializando Armazenamento...");
    fdb_err_t result;

    if (s_lock == NULL) {
        s_lock = xSemaphoreCreateCounting(1, 1);
        assert(s_lock != NULL);
    }

    struct fdb_default_kv default_kv;

    default_kv.kvs = sto_config_table;
    default_kv.num = sizeof(sto_config_table) / sizeof(sto_config_table[0]);
    /* set the lock and unlock function if you want */
    fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_LOCK, lock);
    fdb_kvdb_control(&kvdb, FDB_KVDB_CTRL_SET_UNLOCK, unlock);
    /* Key-Value database initialization
     *
     *       &kvdb: database object
     *       "env": database name
     * "fdb_kvdb1": The flash partition name base on FAL. Please make sure it's in FAL partition table.
     *              Please change to YOUR partition name.
     * &default_kv: The default KV nodes. It will auto add to KVDB when first initialize successfully.
     *        NULL: The user data if you need, now is empty.
     */
    result = fdb_kvdb_init(&kvdb, "env", "fdb_kvdb1", &default_kv, NULL);

    if (result != FDB_NO_ERR) {
        ESP_LOGE(TAG, "Storage FAILED!");
    }

    fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_LOCK, lock);
    fdb_tsdb_control(&tsdb, FDB_TSDB_CTRL_SET_UNLOCK, unlock);
    /* Time series database initialization
     *
     *       &tsdb: database object
     *       "log": database name
     * "fdb_tsdb1": The flash partition name base on FAL. Please make sure it's in FAL partition table.
     *              Please change to YOUR partition name.
     *    get_time: The get current timestamp function.
     *         128: maximum length of each log
     *        NULL: The user data if you need, now is empty.
     */
    result = fdb_tsdb_init(&tsdb, "events", "fdb_tsdb1", get_time, 128, NULL);

    if (result != FDB_NO_ERR) {
        ESP_LOGE(TAG, "Storage FAILED!");
    }

    StorageEvent_t event;
    struct fdb_blob blob;

    /* append new log to TSDB */
    storage_set_lote_number(storage_get_lote_number() + 1);
    event.type = EVENT_LOTE_NUM_CHANGED;
    event.value = lote_number;
    fdb_tsl_append(&tsdb, fdb_blob_make(&blob, &event, sizeof(event)));

    storage_set_lote_number(storage_get_lote_number() + 1);
    event.value = lote_number;
    fdb_tsl_append(&tsdb, fdb_blob_make(&blob, &event, sizeof(event)));

    fdb_tsl_iter(&tsdb, query_cb, &tsdb);

    struct tm tm_from = {.tm_year = 1970 - 1900, .tm_mon = 0, .tm_mday = 1, .tm_hour = 0, .tm_min = 0, .tm_sec = 0};
    struct tm tm_to = {.tm_year = 2020 - 1900, .tm_mon = 4, .tm_mday = 5, .tm_hour = 0, .tm_min = 0, .tm_sec = 0};
    time_t from_time = mktime(&tm_from), to_time = mktime(&tm_to);
    size_t count;
    /* query all TSL in TSDB by time */
    fdb_tsl_iter_by_time(&tsdb, from_time, to_time, query_by_time_cb, &tsdb);
    /* query all FDB_TSL_WRITE status TSL's count in TSDB by time */
    count = fdb_tsl_query_count(&tsdb, from_time, to_time, FDB_TSL_WRITE);

    fdb_tsl_iter(&tsdb, set_status_cb, &tsdb);
}

static bool query_cb(fdb_tsl_t tsl, void *arg) {
    struct fdb_blob blob;
    StorageEvent_t event;
    fdb_tsdb_t db = arg;

    fdb_blob_read((fdb_db_t)db, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, &event, sizeof(event))));
    ESP_LOGI(TAG, "[query_cb] queried a TSL: time: %d, type: %d, value: %d\n", tsl->time, event.type, event.value);
    return false;
}

static bool query_by_time_cb(fdb_tsl_t tsl, void *arg) {
    struct fdb_blob blob;
    StorageEvent_t event;
    fdb_tsdb_t db = arg;

    fdb_blob_read((fdb_db_t)db, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, &event, sizeof(event))));

    return false;
}

static bool set_status_cb(fdb_tsl_t tsl, void *arg) {
    fdb_tsdb_t db = arg;

    fdb_tsl_set_status(db, tsl, FDB_TSL_USER_STATUS1);

    return false;
}

// #define NAMESPACE "config"

// #define LOTE_KEY "lote"

// #define PL_KEY "palha_lenha"

// #define ENTR_MIN_KEY "entr_min"
// #define ENTR_MAX_KEY "entr_max"

// #define M1_MIN_KEY "m1_min"
// #define M1_MAX_KEY "m1_max"

// #define M2_MIN_KEY "m2_min"
// #define M2_MAX_KEY "m2_max"

// #define M3_MIN_KEY "m3_min"
// #define M3_MAX_KEY "m3_max"

// #define M4_MIN_KEY "m4_min"
// #define M4_MAX_KEY "m4_max"

// static const char *TAG = "STORAGE";

// static nvs_handle_t nvsHandle;

// static bool get_bool(const char* key, uint8_t base_value);
// static uint8_t get_u8(const char* key, uint8_t base_value);

// extern QueueHandle_t state_manager_q;

// void storage_init() {
//     // Ler valores nvs
//     ESP_ERROR_CHECK(nvs_open(NAMESPACE, NVS_READWRITE, &nvsHandle));

//     esp_vfs_spiffs_conf_t conf_storage = {
//         .base_path = "/storage",
//         .partition_label = "storage",
//         .max_files = 5,
//         .format_if_mount_failed = true};

//     esp_vfs_spiffs_conf_t conf_website = {
//         .base_path = "/website",
//         .partition_label = "website",
//         .max_files = 5,
//         .format_if_mount_failed = true};

//     // Use settings defined above to initialize and mount SPIFFS filesystem.
//     // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
//     esp_err_t ret_storage = esp_vfs_spiffs_register(&conf_storage);

//     if (ret_storage != ESP_OK) {
//         if (ret_storage == ESP_FAIL) {
//             ESP_LOGE(TAG, "Failed to mount or format filesystem");
//         } else if (ret_storage == ESP_ERR_NOT_FOUND) {
//             ESP_LOGE(TAG, "Failed to find SPIFFS partition");
//         } else {
//             ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret_storage));
//         }
//         return;
//     }

//     esp_err_t ret_website = esp_vfs_spiffs_register(&conf_website);

//     if (ret_website != ESP_OK) {
//         if (ret_website == ESP_FAIL) {
//             ESP_LOGE(TAG, "Failed to mount or format filesystem");
//         } else if (ret_website == ESP_ERR_NOT_FOUND) {
//             ESP_LOGE(TAG, "Failed to find SPIFFS partition");
//         } else {
//             ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret_website));
//         }
//         return;
//     }

//     queimadorMode = get_bool(PL_KEY, false);
//     loteNumber = get_u8(LOTE_KEY, 1);
//     isQueimadorOn = false;
//     isAlarmeOn = false;

//     isM1Connected = false;
//     isM2Connected = false;
//     isM3Connected = false;
//     isM4Connected = false;

//     isAwareEntr = false;
//     isAwareM1 = false;
//     isAwareM2 = false;
//     isAwareM3 = false;
//     isAwareM4 = false;

//     tempEntr = 0;
//     tempM1 = 0;
//     tempM2 = 0;
//     tempM3 = 0;
//     tempM4 = 0;

//     minTempEntr = get_u8(ENTR_MIN_KEY, 0);
//     maxTempEntr = get_u8(ENTR_MAX_KEY, 100);

//     minTempM1 = get_u8(M1_MIN_KEY, 0);
//     maxTempM1 = get_u8(M1_MAX_KEY, 100);

//     minTempM2 = get_u8(M2_MIN_KEY, 0);
//     maxTempM2 = get_u8(M2_MAX_KEY, 100);

//     minTempM3 = get_u8(M3_MIN_KEY, 0);
//     maxTempM3 = get_u8(M3_MAX_KEY, 100);

//     minTempM4 = get_u8(M4_MIN_KEY, 0);
//     maxTempM4 = get_u8(M4_MAX_KEY, 100);

//     sqlite3 *db_conn;
//     sqlite3_open("events.db", &db_conn);
//     sqlite3_close(db_conn);

//     xQueueSend(state_manager_q, INIT_STORAGE, portMAX_DELAY);
// }

// void add_record() {
// }

// static bool get_bool(const char* key, uint8_t base_value) {
//     uint8_t temp_out;

//     esp_err_t err = nvs_get_u8(nvsHandle, key, &temp_out);
//     if (err != ESP_OK) {
//         nvs_set_u8(nvsHandle, key, base_value);

//         if (base_value == 0) {
//             return false;
//         } else {
//             return true;
//         }
//     } else {
//         if (temp_out == 0) {
//             return false;
//         } else {
//             return true;
//         }
//     }
// }

// static uint8_t get_u8(const char* key, uint8_t base_value) {
//     uint8_t temp_out;

//     esp_err_t err = nvs_get_u8(nvsHandle, key, &temp_out);
//     if (err != ESP_OK) {
//         nvs_set_u8(nvsHandle, key, base_value);
//         return base_value;
//     }

//     return temp_out;
// }

// static void set_bool(const char* key, bool new_value) {
//     if (new_value) {
//         ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, key, 1));
//     } else {
//         ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, key, 0));
//     }
// }

// static void set_u8(const char* key, uint8_t new_value) {
//     ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, key, new_value));
// }

// bool get_queimador_mode() {
//     return queimadorMode;
// }

// bool get_queimador_state() {
//     return isQueimadorOn;
// }

// bool get_alarme_state() {
//     return isAlarmeOn;
// }

// bool get_m1_connected() {
//     return isM1Connected;
// }

// bool get_m2_connected() {
//     return isM1Connected;
// }

// bool get_m3_connected() {
//     return isM1Connected;
// }

// bool get_m4_connected() {
//     return isM1Connected;
// }

// bool get_entr_aware() {
//     return isAwareEntr;
// }

// bool get_m1_aware() {
//     return isAwareEntr;
// }

// bool get_m2_aware() {
//     return isAwareEntr;
// }

// bool get_m3_aware() {
//     return isAwareEntr;
// }

// bool get_m4_aware() {
//     return isAwareEntr;
// }

// uint8_t get_temp_entr() {
//     return tempEntr;
// }

// uint8_t get_temp_m1() {
//     return tempM1;
// }

// uint8_t get_temp_m2() {
//     return tempM2;
// }

// uint8_t get_temp_m3() {
//     return tempM3;
// }

// uint8_t get_temp_m4() {
//     return tempM4;
// }

// uint8_t get_min_temp_entr() {
//     return minTempEntr;
// }

// uint8_t get_max_temp_entr() {
//     return maxTempEntr;
// }

// uint8_t get_min_temp_m1() {
//     return minTempM1;
// }

// uint8_t get_max_temp_m1() {
//     return maxTempM1;
// }

// uint8_t get_min_temp_m2() {
//     return minTempM2;
// }

// uint8_t get_max_temp_m2() {
//     return maxTempM2;
// }

// uint8_t get_min_temp_m3() {
//     return minTempM3;
// }

// uint8_t get_max_temp_m3() {
//     return maxTempM3;
// }

// uint8_t get_min_temp_m4() {
//     return minTempM4;
// }

// uint8_t get_max_temp_m4() {
//     return maxTempM4;
// }

// uint8_t get_lote_number() {
//     return loteNumber;
// }

// void set_queimador_mode(bool new_value) {
//     if (get_queimador_mode() != new_value) {
//         set_bool(PL_KEY, new_value);
//         queimadorMode = new_value;
//     }
// }

// void set_queimador_state(bool new_value) {
//     if (get_queimador_state() != new_value) {
//         isQueimadorOn = new_value;
//     }
// }

// void set_alarme_state(bool new_value) {
//     if (get_alarme_state() != new_value) {
//         isAlarmeOn = new_value;
//     }
// }

// void set_m1_connected(bool new_value) {
//     if (get_m1_connected() != new_value) {
//         isM1Connected = new_value;
//     }
// }

// void set_m2_connected(bool new_value) {
//     if (get_m2_connected() != new_value) {
//         isM2Connected = new_value;
//     }
// }

// void set_m3_connected(bool new_value) {
//     if (get_m3_connected() != new_value) {
//         isM3Connected = new_value;
//     }
// }

// void set_m4_connected(bool new_value) {
//     if (get_m4_connected() != new_value) {
//         isM4Connected = new_value;
//     }
// }

// void set_entr_aware(bool new_value) {
//     if (get_entr_aware() != new_value) {
//         isAwareEntr = new_value;
//     }
// }

// void set_m1_aware(bool new_value) {
//     if (get_m1_aware() != new_value) {
//         isAwareM1 = new_value;
//     }
// }

// void set_m2_aware(bool new_value) {
//     if (get_m2_aware() != new_value) {
//         isAwareM2 = new_value;
//     }
// }

// void set_m3_aware(bool new_value) {
//     if (get_m3_aware() != new_value) {
//         isAwareM3 = new_value;
//     }
// }

// void set_m4_aware(bool new_value) {
//     if (get_m4_aware() != new_value) {
//         isAwareM4 = new_value;
//     }
// }

// void set_temp_entr(uint8_t new_value) {
//     if (get_temp_entr() != new_value) {
//         tempEntr = new_value;
//     }
// }

// void set_temp_m1(uint8_t new_value) {
//     if (get_temp_m1() != new_value) {
//         tempM1 = new_value;
//     }
// }

// void set_temp_m2(uint8_t new_value) {
//     if (get_temp_m2() != new_value) {
//         tempM2 = new_value;
//     }
// }

// void set_temp_m3(uint8_t new_value) {
//     if (get_temp_m3() != new_value) {
//         tempM3 = new_value;
//     }
// }

// void set_temp_m4(uint8_t new_value) {
//     if (get_temp_m4() != new_value) {
//         tempM4 = new_value;
//     }
// }

// void set_min_temp_entr(uint8_t new_value) {
//     if (get_min_temp_entr() != new_value) {
//         set_u8(ENTR_MIN_KEY, new_value);
//         minTempEntr = new_value;
//     }
// }

// void set_max_temp_entr(uint8_t new_value) {
//     if (get_max_temp_entr() != new_value) {
//         set_u8(ENTR_MAX_KEY, new_value);
//         maxTempEntr = new_value;
//     }
// }

// void set_min_temp_m1(uint8_t new_value) {
//     if (get_min_temp_m1() != new_value) {
//         set_u8(M1_MIN_KEY, new_value);
//         minTempM1 = new_value;
//     }
// }

// void set_max_temp_m1(uint8_t new_value) {
//     if (get_max_temp_m1() != new_value) {
//         set_u8(M1_MAX_KEY, new_value);
//         maxTempM1 = new_value;
//     }
// }

// void set_min_temp_m2(uint8_t new_value) {
//     if (get_min_temp_m2() != new_value) {
//         set_u8(M2_MIN_KEY, new_value);
//         minTempM2 = new_value;
//     }
// }

// void set_max_temp_m2(uint8_t new_value) {
//     if (get_max_temp_m2() != new_value) {
//         set_u8(M2_MAX_KEY, new_value);
//         maxTempM2 = new_value;
//     }
// }

// void set_min_temp_m3(uint8_t new_value) {
//     if (get_min_temp_m3() != new_value) {
//         set_u8(M3_MIN_KEY, new_value);
//         minTempM3 = new_value;
//     }
// }

// void set_max_temp_m3(uint8_t new_value) {
//     if (get_max_temp_m3() != new_value) {
//         set_u8(M3_MAX_KEY, new_value);
//         maxTempM3 = new_value;
//     }
// }

// void set_min_temp_m4(uint8_t new_value) {
//     if (get_min_temp_m4() != new_value) {
//         set_u8(M4_MIN_KEY, new_value);
//         minTempM4 = new_value;
//     }
// }

// void set_max_temp_m4(uint8_t new_value) {
//     if (get_max_temp_m4() != new_value) {
//         set_u8(M4_MAX_KEY, new_value);
//         maxTempM4 = new_value;
//     }
// }

// void set_lote_number(uint8_t new_value) {
//     if (get_lote_number() != new_value) {
//         set_u8(LOTE_KEY, new_value);
//         loteNumber = new_value;
//     }
// }