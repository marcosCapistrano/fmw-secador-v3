#include "state_manager.h"

#include "stdbool.h"
#include "common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

typedef enum {
    INIT,
    NEW_DRY,
    CONTINUE_DRY,
    DRYING,
} State_t;

typedef struct {
    bool storage;
    bool perif;
    bool ihm;
    bool server;
} InitConditions_t;

static State_t curr_state;
static InitConditions_t init_conditions;

extern QueueHandle_t state_manager_q;

static void process_init_conditions(StateMessage_t state_msg);

static void state_manager_task(void *pvParameters) {
    StateMessage_t state_msg;

    for (;;) {
        if (xQueueReceive(state_manager_q, &state_msg, portMAX_DELAY)) {
            switch (curr_state) {
                case INIT: {
                    process_init_conditions(state_msg);
                    break;
                }
                
                case NEW_DRY:

                break;

                case CONTINUE_DRY:

                break;

                case DRYING:

                break;
            }
        }
    }
}

void state_manager_start_task() {
    xTaskCreate(state_manager_task, "STATE_MANAGER_TASK", 2400, NULL, 5, NULL);
}

void state_manager_init(void) {
    curr_state = INIT;
    init_conditions = (InitConditions_t) {false, false, false, false};
    // Salvar horário de ligamento
}

static void process_init_conditions(StateMessage_t state_msg) {
    switch (state_msg) {
        case INIT_STORAGE:
                init_conditions.storage = true;
            break;
        
        case INIT_PERIF: 
                init_conditions.perif = true;
            break;

        case INIT_IHM:
                init_conditions.ihm = true;
                break;
            
        case INIT_SERVER:
                init_conditions.server=  true;
    }

    if (init_conditions.storage &&
        init_conditions.perif &&
        init_conditions.ihm &&
        init_conditions.server) {
            //se storage get continue dry == 1
            //state = continuedry
            //else
            //state = newdry
    }
}