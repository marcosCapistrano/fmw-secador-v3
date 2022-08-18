#ifndef COMMON_H
#define COMMON_H

enum QueimadorMode_t {
    PALHA,
    LENHA
};

typedef enum {
    IHM_UPDATE_TYPE_INIT,
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

#endif
