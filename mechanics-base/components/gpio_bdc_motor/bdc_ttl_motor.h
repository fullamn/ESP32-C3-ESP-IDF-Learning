#pragma once

#include "esp_err.h"

typedef struct mechanics_bdc_ttl_motor_t *mechanics_bdc_ttl_motor_handle_t;
typedef struct mechanics_bdc_ttl_motor_t mechanics_bdc_ttl_motor_t;
typedef struct mechanics_bdc_ttl_motor_config_t mechanics_bdc_ttl_motor_config_t;

struct mechanics_bdc_ttl_motor_config_t
{
    gpio_num_t clw_pin; // Pin that causes motor to spin clockwise when exclusively high
    gpio_num_t ccw_pin; // Pin that causes motor to spin counter-clockwise when exclusively high
};

struct mechanics_bdc_ttl_motor_t
{
    gpio_num_t clw_pin; // Pin that causes motor to spin clockwise when exclusively high
    gpio_num_t ccw_pin; // Pin that causes motor to spin counter-clockwise when exclusively high
};

esp_err_t mechanics_new_bdc_ttl_motor(const mechanics_bdc_ttl_motor_config_t *config, mechanics_bdc_ttl_motor_handle_t *ret_ttl_motor);

esp_err_t mechanics_run_ttl_motor_clw(mechanics_bdc_ttl_motor_handle_t motor);

esp_err_t mechanics_run_ttl_motor_ccw(mechanics_bdc_ttl_motor_handle_t motor);

esp_err_t mechanics_stop_ttl_motor(mechanics_bdc_ttl_motor_handle_t motor);

esp_err_t mechanics_run_ttl_motor_clw_timed(mechanics_bdc_ttl_motor_handle_t motor, uint32_t msDelay);

esp_err_t mechanics_run_ttl_motor_ccw_timed(mechanics_bdc_ttl_motor_handle_t motor, uint32_t msDelay);