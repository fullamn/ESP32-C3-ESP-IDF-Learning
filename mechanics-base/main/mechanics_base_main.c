/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/sdm.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#define DIR_GPIO_NUM CONFIG_MECHANICS_DIR_GPIO_NUM
#define SPD_GPIO_NUM CONFIG_MECHANICS_SDM_GPIO_NUM
#define SDM_DUTY_MAX(dir) (dir? INT8_MIN:INT8_MAX) 
#define SDM_DUTY_MIN(dir) (dir? -110:87)
#define SDM_DUTY_OFF(dir) (dir? INT8_MAX:INT8_MIN)


static const char *TAG = "mechanics_base";
static uint8_t motor_dir = 0;

int8_t calculate_sdm_density(int8_t dir, int8_t duty){
    int8_t density = (int8_t)((SDM_DUTY_MAX(dir)-SDM_DUTY_MIN(dir))*(duty/100.0)+SDM_DUTY_MIN(dir));
    return density;
}

int8_t get_default_sdm_density(int8_t dir){
    return calculate_sdm_density(dir, CONFIG_MECHANICS_DEFAULT_SDM_DUTY);
}

void app_main(void)
{

    ESP_LOGI(TAG, "Set up direction GPIO");
    gpio_reset_pin(DIR_GPIO_NUM);
    gpio_set_direction(DIR_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(DIR_GPIO_NUM, GPIO_PULLDOWN_ONLY);
    gpio_set_level(DIR_GPIO_NUM, motor_dir);

    ESP_LOGI(TAG, "Install speed sigma delta channel");
    sdm_channel_handle_t sdm_chan = NULL;
    sdm_config_t config = {
        .clk_src = SDM_CLK_SRC_DEFAULT,
        .gpio_num = SPD_GPIO_NUM,
        .sample_rate_hz = 1 * 1000 * 1000, // 1MHz sample rate
    };
    ESP_ERROR_CHECK(sdm_new_channel(&config, &sdm_chan));

    ESP_LOGI(TAG, "Enable sigma delta channel");
    ESP_ERROR_CHECK(sdm_channel_enable(sdm_chan));
    ESP_ERROR_CHECK(sdm_channel_set_pulse_density(sdm_chan, INT8_MIN)); // INT_MIN is a zero value for the pulse

    ESP_LOGI(TAG, "Begin scaling");
    while (1)
    {
        int8_t sdm_density = get_default_sdm_density(motor_dir);
        ESP_LOGI(TAG, "Start at %i", sdm_density);
        ESP_ERROR_CHECK(sdm_channel_set_pulse_density(sdm_chan, sdm_density));
        vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Pause");
        ESP_ERROR_CHECK(sdm_channel_set_pulse_density(sdm_chan, SDM_DUTY_OFF(motor_dir)));
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(TAG, "Flipping");
        motor_dir = !motor_dir;
        gpio_set_level(DIR_GPIO_NUM,motor_dir);
        ESP_ERROR_CHECK(sdm_channel_set_pulse_density(sdm_chan, SDM_DUTY_OFF(motor_dir)));

    }
}
