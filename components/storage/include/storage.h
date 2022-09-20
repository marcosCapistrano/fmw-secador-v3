#ifndef STORAGE_H
#define STORAGE_H

#include <inttypes.h>
#include <stdbool.h>

void storage_init();

uint8_t storage_get_lote_number();
void storage_set_lote_number(uint8_t new_value);

bool storage_get_lote_concluded();
void storage_set_lote_concluded(bool new_value);

bool storage_get_queimador_mode();
void storage_set_queimador_mode(bool new_value);

bool storage_get_queimador_state();
void storage_set_queimador_state(bool new_value);

uint8_t storage_get_sensor_entr();
void storage_set_sensor_entr(uint8_t new_value);

uint8_t storage_get_sensor_m1();
void storage_set_sensor_m1(uint8_t new_value);

uint8_t storage_get_sensor_m2();
void storage_set_sensor_m2(uint8_t new_value);

uint8_t storage_get_sensor_m3();
void storage_set_sensor_m3(uint8_t new_value);

uint8_t storage_get_sensor_m4();
void storage_set_sensor_m4(uint8_t new_value);

bool storage_get_alarme_entr();
void storage_set_alarme_entr(bool new_value);

bool storage_get_alarme_m1();
void storage_set_alarme_m1(bool new_value);

bool storage_get_alarme_m2();
void storage_set_alarme_m2(bool new_value);

bool storage_get_alarme_m3();
void storage_set_alarme_m3(bool new_value);

bool storage_get_alarme_m4();
void storage_set_alarme_m4(bool new_value);

uint8_t storage_get_min_entr();
void storage_set_min_entr(uint8_t new_value);

uint8_t storage_get_max_entr();
void storage_set_max_entr(uint8_t new_value);

uint8_t storage_get_min_m1();
void storage_set_min_m1(uint8_t new_value);

uint8_t storage_get_max_m1();
void storage_set_max_m1(uint8_t new_value);

uint8_t storage_get_min_m2();
void storage_set_min_m2(uint8_t new_value);

uint8_t storage_get_max_m2();
void storage_set_max_m2(uint8_t new_value);

uint8_t storage_get_min_m3();
void storage_set_min_m3(uint8_t new_value);

uint8_t storage_get_max_m3();
void storage_set_max_m3(uint8_t new_value);

uint8_t storage_get_min_m4();
void storage_set_min_m4(uint8_t new_value);
uint8_t storage_get_max_m4();
void storage_set_max_m4(uint8_t new_value);

bool storage_get_conexao_m1();
void storage_set_conexao_m1(bool new_value);

bool storage_get_conexao_m2();
void storage_set_conexao_m2(bool new_value);

bool storage_get_conexao_m3();
void storage_set_conexao_m3(bool new_value);

bool storage_get_conexao_m4();
void storage_set_conexao_m4(bool new_value);

typedef enum {
    EVENT_SENSOR_ENTR,
    EVENT_SENSOR_M1,
    EVENT_SENSOR_M2,
    EVENT_SENSOR_M3,
    EVENT_SENSOR_M4,

    EVENT_FINISHED,

    EVENT_ALARME_ENTRADA,
    EVENT_ALARME_M1,
    EVENT_ALARME_M2,
    EVENT_ALARME_M3,
    EVENT_ALARME_M4,
    EVENT_QUEIMADOR,

    EVENT_CONEXAO_M1,
    EVENT_CONEXAO_M2,
    EVENT_CONEXAO_M3,
    EVENT_CONEXAO_M4,
} StorageEventType_t;

void storage_add_record_init();
void storage_add_record_device_on();
void storage_start_new_lote();
void storage_add_record_generic(StorageEventType_t type, void *payload);


#endif