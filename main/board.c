/* board.c - Board-specific hooks */

/*
 * Copyright (c) 2017 Intel Corporation
 * Additional Copyright (c) 2018 Espressif Systems (Shanghai) PTE LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "board.h"
#include "loop.h"



#define TAG "BOARD"
#define TIMER_GRANUL_MS 10
//**********************************************************************************
void LED_operation(uint8_t onoff){
    gpio_set_level(LED_B, onoff);
}
void LED_toggle(){
    gpio_set_level(LED_B,!gpio_get_level(LED_B));
}

static void board_led_init(void)
{
    gpio_reset_pin(LED_B);
    gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_B, LED_OFF);
}
void board_init(void) {
    board_led_init();
}
//****************************************************************************
static volatile uint8_t button_was_pressed=0;
uint8_t IsButtonPressed(void){
    uint8_t res=button_was_pressed;
    if(res){
        button_was_pressed=0;
    }
    return res;
}
//****************************************
typedef struct{
    uint32_t first_tk_sw_states; //shift register of state of first tank level overflow switch 
    uint32_t drain_tk_sw_states; //shift register of state of drain tank overfolow switch 
    uint32_t button_state;//shift register of button
}tDigIn_st;
static tDigIn_st dig_ins={
    .first_tk_sw_states=0x00,
    .drain_tk_sw_states=0x00,
    .button_state=0xff,
};
//read/write 32bit value supposed to be atomic, so we use this value to avoid using semaphore
volatile uint32_t dig_in_snap=0;

void DigInLoop(void* tmr) ;
//**********************************************************************************
void DigInInit(void){
    //inputs 
    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1ULL<<BUTTON_PIN) | (1ULL<<FIRST_TK_LEVEL_SWT_IN_PIN) | (1ULL<<DRAIN_TK_LEVEL_SWT_IN_PIN);
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&gpio_conf);
    TimerHandle_t tmr=xTimerCreate("dig_in_tmr",pdMS_TO_TICKS(TIMER_GRANUL_MS),pdTRUE,NULL,DigInLoop);
    xTimerStart(tmr,0);
}
void DigInLoop(void* tmr) {
    uint32_t snap=0;
    dig_ins.first_tk_sw_states = (dig_ins.first_tk_sw_states << 1) | gpio_get_level(FIRST_TK_LEVEL_SWT_IN_PIN);
    dig_ins.drain_tk_sw_states = (dig_ins.drain_tk_sw_states << 1) | gpio_get_level(DRAIN_TK_LEVEL_SWT_IN_PIN);
    dig_ins.button_state = (dig_ins.button_state << 1) | gpio_get_level(BUTTON_PIN);
    if((dig_ins.button_state&0x07)==0x04)button_was_pressed=1;
    if((dig_ins.first_tk_sw_states & 0x07) == 0x07)
        snap|=SEPTIC_ALARM_FIRST_TANK_OVF;
    if((dig_ins.drain_tk_sw_states & 0x07) == 0x07)  // some of level overfill arised
       snap|=SEPTIC_ALARM_DRAIN_TANK_OVF;
    dig_in_snap=snap;
}
//**********************************************************************************
