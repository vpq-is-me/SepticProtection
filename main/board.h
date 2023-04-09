/* board.h - Board-specific hooks */

/*
 * Copyright (c) 2017 Intel Corporation
 * Additional Copyright (c) 2018 Espressif Systems (Shanghai) PTE LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _BOARD_H_
#define _BOARD_H_

#include "driver/gpio.h"
#include "stdint.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#define LED_ON  1
#define LED_OFF 0

#define LED_B                           GPIO_NUM_2
#define BUTTON_PIN                      GPIO_NUM_23
#define FIRST_TK_LEVEL_SWT_IN_PIN       GPIO_NUM_19
#define DRAIN_TK_LEVEL_SWT_IN_PIN       GPIO_NUM_18
// #define I2C_MASTER_SCL_IO 26        /*!< gpio number for I2C master clock */
// #define I2C_MASTER_SDA_IO 25        /*!< gpio number for I2C master data  */

void LED_operation(uint8_t onoff);
void LED_toggle();
void board_init(void);

void DigInInit(void);
uint8_t IsButtonPressed(void);
extern volatile uint32_t dig_in_snap;

#endif
