#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/sdm.h"
#include "driver/gpio.h"

#include "sdkconfig.h"

#include "bdc_sdm_motor.h"

#define SDM_DUTY_MAX(dir) (dir ? INT8_MIN : INT8_MAX)
#define SDM_DUTY_MIN(dir) (dir ? -76 : 80)
#define SDM_DUTY_OFF(dir) (dir ? INT8_MAX : INT8_MIN)

static const char *TAG = "bdc_sdm_motor";

int8_t calculate_sdm_density(int32_t dir, int8_t duty)
{
    int8_t density = (int8_t)((SDM_DUTY_MAX(dir) - SDM_DUTY_MIN(dir)) * (duty / 100.0) + SDM_DUTY_MIN(dir));
    return density;
}

esp_err_t mechanics_new_bdc_sdm_motor(const mechanics_bdc_sdm_motor_config_t *config, mechanics_bdc_sdm_motor_handle_t *ret_sdm_motor)
{
    esp_err_t ret = ESP_OK;
    mechanics_bdc_sdm_motor_t *motor = NULL;
    ESP_GOTO_ON_FALSE(config && ret_sdm_motor, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    ESP_GOTO_ON_FALSE(GPIO_IS_VALID_OUTPUT_GPIO(config->dir_pin), ESP_ERR_INVALID_ARG, err, TAG, "invalid GPIO number");

    // get memory for motor struct
    motor = heap_caps_calloc(1, sizeof(mechanics_bdc_sdm_motor_t), MALLOC_CAP_DEFAULT);
    ESP_GOTO_ON_FALSE(motor, ESP_ERR_NO_MEM, err, TAG, "no mem for motor");

    // setup gpio
    gpio_config_t gpio_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = true,
        .pull_up_en = false,
        .pin_bit_mask = 1ULL << config->dir_pin};
    ESP_GOTO_ON_ERROR(gpio_config(&gpio_conf), err, TAG, "config GPIO failed");
    ESP_GOTO_ON_ERROR(gpio_set_level(config->dir_pin, 0), err, TAG, "config GPIO failed");
    motor->dir_pin = config->dir_pin;

    // setup and enable sdm channel
    sdm_channel_handle_t sdm_chan = NULL;
    sdm_config_t sdm_config = {
        .clk_src = SDM_CLK_SRC_DEFAULT,
        .gpio_num = config->spd_pin,
        .sample_rate_hz = 1 * 1000 * 1000, // 1MHz sample rate
    };
    ESP_GOTO_ON_ERROR(sdm_new_channel(&sdm_config, &sdm_chan), err, TAG, "config SDM failed");
    ESP_GOTO_ON_ERROR(sdm_channel_enable(sdm_chan), err, TAG, "enable SDM failed");
    ESP_GOTO_ON_ERROR(sdm_channel_set_pulse_density(sdm_chan, INT8_MIN), err, TAG, "initialize SDM pulse density failed");
    motor->spd_ch = sdm_chan;

    // init rest of motor members
    motor->cw_lvl = config->cw_lvl;

    *ret_sdm_motor = motor;
    return ESP_OK;

err:
    if (motor)
    {
        free(motor);
    }
    return ret;
}

esp_err_t mechanics_run_sdm_motor_clw_variable_spd(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty)
{
    ESP_RETURN_ON_FALSE(motor, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->dir_pin, motor->cw_lvl), TAG, "set GPIO to CW level failed");
    ESP_RETURN_ON_ERROR(sdm_channel_set_pulse_density(motor->spd_ch, calculate_sdm_density(motor->cw_lvl, duty)), TAG, "set SDM channel to %i failed", duty);
    return ESP_OK;
}

esp_err_t mechanics_run_sdm_motor_ccw_variable_spd(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty)
{
    ESP_RETURN_ON_FALSE(motor, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->dir_pin, !(motor->cw_lvl)), TAG, "set GPIO to CCW level failed");
    ESP_RETURN_ON_ERROR(sdm_channel_set_pulse_density(motor->spd_ch, calculate_sdm_density(!(motor->cw_lvl), duty)), TAG, "set SDM channel to %i failed", duty);
    return ESP_OK;
}

esp_err_t mechanics_stop_sdm_motor(mechanics_bdc_sdm_motor_handle_t motor)
{
    ESP_RETURN_ON_FALSE(motor, ESP_ERR_INVALID_ARG, TAG, "invalid argument");
    ESP_RETURN_ON_ERROR(gpio_set_level(motor->dir_pin, 0), TAG, "set GPIO level to stop motor failed");
    ESP_RETURN_ON_ERROR(sdm_channel_set_pulse_density(motor->spd_ch, INT8_MIN), TAG, "set SDM channel to stop motor failed");
    return ESP_OK;
}

esp_err_t mechanics_run_sdm_motor_clw_variable_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty, uint32_t msDelay)
{
    mechanics_run_sdm_motor_clw_variable_spd(motor, duty);
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    mechanics_stop_sdm_motor(motor);
    return ESP_OK;
}

esp_err_t mechanics_run_sdm_motor_ccw_variable_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty, uint32_t msDelay)
{
    mechanics_run_sdm_motor_ccw_variable_spd(motor, duty);
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    mechanics_stop_sdm_motor(motor);
    return ESP_OK;
}

esp_err_t mechanics_run_sdm_motor_clw_default_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint32_t msDelay)
{
    mechanics_run_sdm_motor_clw_variable_spd(motor, motor->default_duty);
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    mechanics_stop_sdm_motor(motor);
    return ESP_OK;
}

esp_err_t mechanics_run_sdm_motor_ccw_default_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint32_t msDelay)
{
    mechanics_run_sdm_motor_ccw_variable_spd(motor, motor->default_duty);
    vTaskDelay(pdMS_TO_TICKS(msDelay));
    mechanics_stop_sdm_motor(motor);
    return ESP_OK;
}