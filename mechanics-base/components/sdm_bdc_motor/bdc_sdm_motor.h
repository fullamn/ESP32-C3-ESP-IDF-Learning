#pragma once

#include "esp_err.h"

typedef struct mechanics_bdc_sdm_motor_t *mechanics_bdc_sdm_motor_handle_t;
typedef struct mechanics_bdc_sdm_motor_t mechanics_bdc_sdm_motor_t;
typedef struct mechanics_bdc_sdm_motor_config_t mechanics_bdc_sdm_motor_config_t;

struct mechanics_bdc_sdm_motor_config_t
{
    gpio_num_t spd_pin;   // SDM Pin number to control speed
    gpio_num_t dir_pin;   // GPIO Pin number to control direction
    uint32_t cw_lvl;      // Logic level for dir_pin that drives the motor clockwise
    uint8_t default_duty; // Default duty cycle to use
};

struct mechanics_bdc_sdm_motor_t
{
    sdm_channel_handle_t spd_ch; // SDM Channel number to control speed
    gpio_num_t dir_pin;          // GPIO Pin number to control direction
    uint32_t cw_lvl;             // Logic level for dir_pin that drives the motor clockwise
    uint8_t default_duty;        // Default duty cycle to use
};

int8_t calculate_sdm_density(int32_t dir, int8_t duty);

esp_err_t mechanics_new_bdc_sdm_motor(const mechanics_bdc_sdm_motor_config_t *config, mechanics_bdc_sdm_motor_handle_t *ret_sdm_motor);

esp_err_t mechanics_run_sdm_motor_clw_variable_spd(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty);

esp_err_t mechanics_run_sdm_motor_ccw_variable_spd(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty);

esp_err_t mechanics_stop_sdm_motor(mechanics_bdc_sdm_motor_handle_t motor);

esp_err_t mechanics_run_sdm_motor_clw_variable_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty, uint32_t msDelay);

esp_err_t mechanics_run_sdm_motor_ccw_variable_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint8_t duty, uint32_t msDelay);

esp_err_t mechanics_run_sdm_motor_clw_default_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint32_t msDelay);

esp_err_t mechanics_run_sdm_motor_ccw_default_spd_timed(mechanics_bdc_sdm_motor_handle_t motor, uint32_t msDelay);