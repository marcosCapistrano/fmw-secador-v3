#ifndef STATE_LOGIC_H
#define STATE_LOGIC_H

#include "common.h"

PerifMessageType_t get_perif_alarme_action(void **payload);
PerifMessageType_t get_perif_queimador_action(void **payload);
PerifMessageType_t get_perif_led_entr_q_action(void **payload);
PerifMessageType_t get_perif_led_entr_f_action(void **payload);
PerifMessageType_t get_perif_led_mass_q_action(void **payload);
PerifMessageType_t get_perif_led_mass_f_action(void **payload);
PerifMessageType_t get_perif_connection_action(void **payload);

IHMMessageType_t get_ihm_entr_action(void **payload);
IHMMessageType_t get_ihm_m1_action(void **payload);
IHMMessageType_t get_ihm_m2_action(void **payload);
IHMMessageType_t get_ihm_m3_action(void **payload);
IHMMessageType_t get_ihm_m4_action(void **payload);


bool is_entr_within_limits();
bool is_m1_within_limits();
bool is_m2_within_limits();
bool is_m3_within_limits();
bool is_m4_within_limits();

#endif