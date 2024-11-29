#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "bdc_ttl_motor.h"

static const char *TAG = "gpio_bdc_ttl_motor";

esp_err_t mechanics_new_bdc_ttl_motor(const mechanics_bdc_ttl_motor_config_t *config, mechanics_bdc_ttl_motor_handle_t *ret_ttl_motor)
{
    esp_err_t ret = ESP_OK;
    mechanics_bdc_ttl_motor_t *motor = NULL;
    ESP_GOTO_ON_FALSE(config && ret_ttl_motor, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    ESP_GOTO_ON_FALSE(GPIO_IS_VALID_OUTPUT_GPIO(config->clw_pin), ESP_ERR_INVALID_ARG, err, TAG, "invalid GPIO number for CLW pin");
    ESP_GOTO_ON_FALSE(GPIO_IS_VALID_OUTPUT_GPIO(config->ccw_pin), ESP_ERR_INVALID_ARG, err, TAG, "invalid GPIO number for CCW pin");

    // get memory for motor struct
    motor = heap_caps_calloc(1, sizeof(mechanics_bdc_ttl_motor_t), MALLOC_CAP_DEFAULT);
    ESP_GOTO_ON_FALSE(motor, ESP_ERR_NO_MEM, err, TAG, "no mem for motor");

    // setup gpio on clw pin
    gpio_config_t clw_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = true,
        .pull_up_en = false,
        .pin_bit_mask = 1ULL << config->clw_pin};
    ESP_GOTO_ON_ERROR(gpio_config(&clw_conf), err, TAG, "config clw failed");
    ESP_GOTO_ON_ERROR(gpio_set_level(config->clw_pin, 0), err, TAG, "config clw failed");
    motor->clw_pin = config->clw_pin;

    // setup gpio on ccw pin
    gpio_config_t ccw_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = true,
        .pull_up_en = false,
        .pin_bit_mask = 1ULL << config->ccw_pin};
    ESP_GOTO_ON_ERROR(gpio_config(&ccw_conf), err, TAG, "config ccw failed");
    ESP_GOTO_ON_ERROR(gpio_set_level(config->ccw_pin, 0), err, TAG, "config GPIO failed");
    motor->ccw_pin = config->ccw_pin;

    *ret_ttl_motor = motor;
    return ESP_OK;

err:
    if (motor)
    {
        free(motor);
    }
    return ret;
}

esp_err_t mechanics_run_ttl_motor_clw(mechanics_bdc_ttl_motor_handle_t motor)
{
    ESP_RETURN_ON_FALSE(motor, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->ccw_pin, 0), TAG, "set CCW Pin to LOW failed");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->clw_pin, 1), TAG, "set CLW Pin to HIGH failed");
    return ESP_OK;
}

esp_err_t mechanics_run_ttl_motor_ccw(mechanics_bdc_ttl_motor_handle_t motor)
{
    ESP_RETURN_ON_FALSE(motor, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->clw_pin, 0), TAG, "set CLW Pin to LOW failed");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->ccw_pin, 1), TAG, "set CCW Pin to HIGH failed");
    return ESP_OK;
}

esp_err_t mechanics_stop_ttl_motor(mechanics_bdc_ttl_motor_handle_t motor)
{
    ESP_RETURN_ON_FALSE(motor, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->clw_pin, 0), TAG, "set CLW pin to LOW to stop motor failed");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->ccw_pin, 0), TAG, "set CCW pin to LOW to stop motor failed");
    return ESP_OK;
}

esp_err_t mechanics_run_ttl_motor_clw_timed(mechanics_bdc_ttl_motor_handle_t motor, uint32_t msDelay)
{
    mechanics_run_ttl_motor_clw(motor);
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    mechanics_stop_ttl_motor(motor);
    return ESP_OK;
}

esp_err_t mechanics_run_ttl_motor_ccw_timed(mechanics_bdc_ttl_motor_handle_t motor, uint32_t msDelay)
{
    mechanics_run_ttl_motor_ccw(motor);
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    mechanics_stop_ttl_motor(motor);
    return ESP_OK;
}