/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/sdm.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "autoblinds_mechanics.h"

#define DIR_GPIO_NUM CONFIG_MECHANICS_DIR_GPIO_NUM
#define SPD_GPIO_NUM CONFIG_MECHANICS_SDM_GPIO_NUM

static const char *TAG = "mechanics_base";

void app_main(void)
{
    // Setup motor
    ESP_LOGI(TAG, "Set up motor struct");
    mechanics_bdc_motor_handle_t motor = NULL;
    mechanics_bdc_motor_config_t config = {
        .cw_lvl = CONFIG_MECHANICS_CW_LVL,
        .default_duty = CONFIG_MECHANICS_DEFAULT_SDM_DUTY,
        .dir_pin = DIR_GPIO_NUM,
        .spd_pin = SPD_GPIO_NUM};
    mechanics_new_bdc_motor(&config, &motor);
    uint8_t step = 5;
    uint8_t duty = 5;
    while (1)
    {
        ESP_LOGI(TAG, "Run CW at %i", duty);
        mechanics_run_motor_cw_timed(motor, duty, 2000);
        ESP_LOGI(TAG, "STOP");
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Run CCW at %i", duty);
        mechanics_run_motor_ccw_timed(motor, duty, 2000);
        ESP_LOGI(TAG, "STOP");
        vTaskDelay(pdMS_TO_TICKS(2000));
        if(duty==100||duty==0){
            step *= -1;
        }
        duty += step;
    }
}
