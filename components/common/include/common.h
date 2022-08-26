#ifndef COMMON_H
#define COMMON_H

#define IHM_INPUT_QUEUE_LEN 5
#define IHM_UPDATE_QUEUE_LEN 5
#define IHM_QUEUE_SET_LEN (IHM_INPUT_QUEUE_LEN + IHM_UPDATE_QUEUE_LEN)

typedef enum {
    IHM_UPDATE_CHANGE_PAGE,
} IHMUpdateType_t;

typedef enum {
    IHM_UPDATE_INIT_STORAGE,
    IHM_UPDATE_INIT_SENSOR_ENTR,
    IHM_UPDATE_INIT_SENSOR_M1,
    IHM_UPDATE_INIT_SENSOR_M2,
    IHM_UPDATE_INIT_SENSOR_M3,
    IHM_UPDATE_INIT_SENSOR_M4,
} IHMUpdateTypeInit_t;

typedef struct {
    IHMUpdateType_t type;
    void *payload;
} IHMUpdate_t;

typedef enum {
    INIT_STORAGE,
    INIT_PERIF,
    INIT_IHM,
    INIT_SERVER
} StateMessage_t;

typedef enum {
    OI
} PerifUpdate_t;

#endif
