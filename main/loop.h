#ifndef LOOP_H
#define LOOP_H


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "main.h"

#define LOOP_STACK_SIZE 4096

void loop_task(void* arg);


#define SEPTIC_ALARM_FIRST_TANK_OVF     (0x01ul << 0)
#define SEPTIC_ALARM_DRAIN_TANK_OVF     (0x01ul << 1)



typedef enum tSeptic_param_i_en{SEPTIC_PARAM_ID_ALARMS_PEND,
                                SEPTIC_PARAM_ID_ALARMS_NACK,
                            }Septic_param_id;
                            
int8_t SepticGetParameter(Septic_param_id id);

int8_t SepticSetParameter(Septic_param_id id, int8_t val);

#endif