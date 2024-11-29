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

static const char *TAG = "mechanics_base";

#if defined(CONFIG_MECHANICS_SDM)
#include "bdc_sdm_motor.h"

#define DIR_GPIO_NUM CONFIG_MECHANICS_DIR_GPIO_NUM
#define SPD_GPIO_NUM CONFIG_MECHANICS_SDM_GPIO_NUM
static const char *STARTUP_STRING = "RUNNING WITH SDM";

void new_motor(const mechanics_bdc_sdm_motor_config_t *config, mechanics_bdc_sdm_motor_handle_t *ret_motor)
{
    mechanics_new_bdc_sdm_motor(config, ret_motor);
}
esp_err_t run_motor_clw(mechanics_bdc_sdm_motor_handle_t motor, uint32_t msDelay)
{
    return mechanics_run_sdm_motor_clw_default_spd_timed(motor, msDelay);
}
esp_err_t run_motor_ccw(mechanics_bdc_sdm_motor_handle_t motor, uint32_t msDelay)
{
    return mechanics_run_sdm_motor_ccw_default_spd_timed(motor, msDelay);
}

const mechanics_bdc_sdm_motor_config_t config = {
    .cw_lvl = CONFIG_MECHANICS_CLW_LVL,
    .default_duty = CONFIG_MECHANICS_DEFAULT_SDM_DUTY,
    .dir_pin = DIR_GPIO_NUM,
    .spd_pin = SPD_GPIO_NUM};

mechanics_bdc_sdm_motor_handle_t motor = NULL;

#elif defined(CONFIG_MECHANICS_TTL)
#include "bdc_ttl_motor.h"

static const char *STARTUP_STRING = "RUNNING WITH TTL";

void new_motor(const mechanics_bdc_ttl_motor_config_t *config, mechanics_bdc_ttl_motor_handle_t *ret_motor)
{
    mechanics_new_bdc_ttl_motor(config, ret_motor);
}
esp_err_t run_motor_clw(mechanics_bdc_ttl_motor_handle_t motor, uint32_t msDelay)
{
    return mechanics_run_ttl_motor_clw_timed(motor, msDelay);
}
esp_err_t run_motor_ccw(mechanics_bdc_ttl_motor_handle_t motor, uint32_t msDelay)
{
    return mechanics_run_ttl_motor_ccw_timed(motor, msDelay);
}

#define PINS(num) (num ? CONFIG_MECHANICS_PIN1_GPIO_NUM : CONFIG_MECHANICS_PIN0_GPIO_NUM)
const mechanics_bdc_ttl_motor_config_t config = {
    .clw_pin = PINS(CONFIG_MECHANICS_CLW_PIN),
    .ccw_pin = PINS(!CONFIG_MECHANICS_CLW_PIN)};

mechanics_bdc_ttl_motor_handle_t motor = NULL;

#else

static const char *STARTUP_STRING = "RUNNING WITH NO MOTOR";
void new_motor(int *config, bool *ret_motor){
    return;
}
esp_err_t run_motor_clw(bool motor, uint32_t msDelay)
{
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    return ESP_OK;
}
esp_err_t run_motor_ccw(bool motor, uint32_t msDelay)
{
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    return ESP_OK;
}
int config = 0;
bool motor = 0;

#endif

void app_main(void)
{
    // Setup motor
    ESP_LOGI(TAG, "%s", STARTUP_STRING);
    ESP_LOGI(TAG, "Set up motor struct");
    new_motor(&config, &motor);
    uint32_t delay = 2000;
    while (1)
    {
        ESP_LOGI(TAG, "Run CLW");
        run_motor_clw(motor, delay);
        ESP_LOGI(TAG, "STOP");
        vTaskDelay(pdMS_TO_TICKS(delay));
        ESP_LOGI(TAG, "Run CCW");
        run_motor_ccw(motor, delay);
        ESP_LOGI(TAG, "STOP");
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}
