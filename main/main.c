// /*
//  * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
//  *
//  * SPDX-License-Identifier: CC0-1.0
//  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define GPIO_BUTTON_PIN         GPIO_NUM_4
#define GPIO_LED_PIN            GPIO_NUM_2
#define ESP_INTR_FLAG_DEFAULT   0

#define DEBOUNCE_DELAY_MS       50

static QueueHandle_t gpio_evt_queue = NULL;

void print_id(void* arg) {
    while(1) {
        printf("2011507\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    } 
}

void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void print_eps32(void* arg)
{
    uint32_t io_num;
    uint32_t current_state;
    uint32_t last_state = 0;
    while(1) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            current_state = gpio_get_level(io_num);
            if (current_state != last_state) {
                vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
                gpio_set_level(GPIO_LED_PIN, gpio_get_level(io_num));
                printf("ESP32\n");
            }
            last_state = current_state;
        }
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {};

    //Config for LED PIN
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL<<GPIO_LED_PIN;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    //Config for INTERRUPT PIN
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = 1ULL<<GPIO_BUTTON_PIN;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //Create Queue
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    
    //Create and Add Task
    xTaskCreate(print_id, "print_id", 2048, NULL, 5, NULL);
    xTaskCreate(print_eps32, "print_eps32", 1024, NULL, 5, NULL);

    //Instal Interrupt Service Routine
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_BUTTON_PIN, gpio_isr_handler, (void*) GPIO_BUTTON_PIN);
}