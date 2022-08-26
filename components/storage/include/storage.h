#ifndef STORAGE_H
#define STORAGE_H

#include <inttypes.h>
#include <stdbool.h>

void storage_init();

bool get_queimador_mode();

bool get_queimador_state();
bool get_alarme_state();

bool get_m1_connected();
bool get_m2_connected();
bool get_m3_connected();
bool get_m4_connected();

bool get_entr_aware();
bool get_m1_aware();
bool get_m2_aware();
bool get_m3_aware();
bool get_m4_aware();

uint8_t get_temp_entr();
uint8_t get_temp_m1();
uint8_t get_temp_m2();
uint8_t get_temp_m3();
uint8_t get_temp_m4();

uint8_t get_min_temp_entr();
uint8_t get_max_temp_entr();

uint8_t get_min_temp_m1();
uint8_t get_max_temp_m1();

uint8_t get_min_temp_m2();
uint8_t get_max_temp_m2();

uint8_t get_min_temp_m3();
uint8_t get_max_temp_m3();

uint8_t get_min_temp_m4();
uint8_t get_max_temp_m4();

void set_queimador_mode(bool new_value);

void set_queimador_state(bool new_value);
void set_alarme_state(bool new_value);

void set_m1_connected(bool new_value);
void set_m2_connected(bool new_value);
void set_m3_connected(bool new_value);
void set_m4_connected(bool new_value);

void set_entr_aware(bool new_value);
void set_m1_aware(bool new_value);
void set_m2_aware(bool new_value);
void set_m3_aware(bool new_value);
void set_m4_aware(bool new_value);

void set_temp_entr(uint8_t new_value);
void set_temp_m1(uint8_t new_value);
void set_temp_m2(uint8_t new_value);
void set_temp_m3(uint8_t new_value);
void set_temp_m4(uint8_t new_value);

void set_min_temp_entr(uint8_t new_value);
void set_max_temp_entr(uint8_t new_value);
void set_min_temp_m1(uint8_t new_value);
void set_max_temp_m1(uint8_t new_value);
void set_min_temp_m2(uint8_t new_value);
void set_max_temp_m2(uint8_t new_value);
void set_min_temp_m3(uint8_t new_value);
void set_max_temp_m3(uint8_t new_value);
void set_min_temp_m4(uint8_t new_value);
void set_max_temp_m4(uint8_t new_value);

#endif