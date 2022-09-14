#ifndef COMMON_H
#define COMMON_H

#define IHM_INPUT_QUEUE_LEN 5
#define IHM_UPDATE_QUEUE_LEN 5
#define IHM_QUEUE_SET_LEN (IHM_INPUT_QUEUE_LEN + IHM_UPDATE_QUEUE_LEN)

#include "freertos/FreeRTOS.h"

typedef enum {
    IHM_MSG_CHANGE_QUEIMADOR_MODE, 
    IHM_MSG_CHANGE_ENTR_LIMITS, 
    IHM_MSG_CHANGE_M1_LIMITS, 
    IHM_MSG_CHANGE_M2_LIMITS, 
    IHM_MSG_CHANGE_M3_LIMITS, 
    IHM_MSG_CHANGE_M4_LIMITS, 
} IHMMessageType_t;

typedef struct {
    IHMMessageType_t type;
    void *payload; 
} IHMMessage_t;

typedef enum {
    SRV_MSG_OI
} ServerMessageType_t;

typedef struct {
   ServerMessageType_t type;
   int value; 
} ServerMessage_t;

typedef enum {
    STA_MSG_CHANGE_QUEIMADOR_MODE,

    STA_MSG_CHANGE_LIMIT_ENTR_MIN,
    STA_MSG_CHANGE_LIMIT_ENTR_MAX,
    STA_MSG_CHANGE_LIMIT_M1_MIN,
    STA_MSG_CHANGE_LIMIT_M1_MAX,
    STA_MSG_CHANGE_LIMIT_M2_MIN,
    STA_MSG_CHANGE_LIMIT_M2_MAX,
    STA_MSG_CHANGE_LIMIT_M3_MIN,
    STA_MSG_CHANGE_LIMIT_M3_MAX,
    STA_MSG_CHANGE_LIMIT_M4_MIN,
    STA_MSG_CHANGE_LIMIT_M4_MAX,
} StateMessageType_t;

typedef struct {
   StateMessageType_t type;
   void *payload;
} StateMessage_t;

typedef enum {
    PRF_MSG_OI
} PerifMessageType_t;

typedef struct {
   PerifMessageType_t type;
   int value; 
} PerifMessage_t;

void common_send_state_msg(StateMessageType_t type, void *payload, portTickType ticks_to_wait);
void common_send_ihm_msg(IHMMessageType_t type, void *payload, portTickType ticks_to_wait);
void common_send_perif_msg(PerifMessageType_t type, void *payload, portTickType ticks_to_wait);
void common_send_server_msg(ServerMessageType_t type, void *payload, portTickType ticks_to_wait);

#endif
