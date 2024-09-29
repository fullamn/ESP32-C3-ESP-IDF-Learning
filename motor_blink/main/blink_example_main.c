/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "esp_random.h"

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;
static uint8_t s_motor_state = 0;
static uint8_t s_motor_high_pin = 0;

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        /* Randomize the color by splitting the rand uint32_t into 4 uint8_t */
        uint32_t *colors[4];
        uint32_t rng = esp_random();
        colors[0] = (rng & 0x000000ff);
        colors[1] = (rng & 0x0000ff00)>>8;
        colors[2] = (rng & 0x00ff0000)>>16;
        colors[4] = (rng & 0xff000000)>>24;
        led_strip_set_pixel(led_strip, 0, colors[0],colors[1],colors[2]);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}
static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif

#ifdef CONFIG_BLINK_MOTOR_INCLUDE

#define MOTOR_PINS(pos) (pos? CONFIG_BLINK_MOTOR_GPIO_2:CONFIG_BLINK_MOTOR_GPIO_1)

static void configure_motor(void){
    ESP_LOGI(TAG,"Configured to 'blink' motor");
    gpio_reset_pin(MOTOR_PINS(true));
    gpio_reset_pin(MOTOR_PINS(false));
    gpio_set_direction(MOTOR_PINS(true), GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_PINS(false), GPIO_MODE_OUTPUT);
}

static void blink_motor(void){
    gpio_set_level(MOTOR_PINS(s_motor_high_pin), s_motor_state);
}
#elif CONFIG_BLINK_MOTOR_IGNORE
    static void configure_motor(void){};
    static void blink_motor(void){};
#else
#error "Unsupported Motor option"
#endif

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();
    configure_motor();

    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Turning the Motor %s: %s!", s_motor_high_pin == true ? "FWD" : "BWD", s_motor_state == true ? "ON": "OFF");
        blink_motor();
        s_motor_state = !s_motor_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        if(s_motor_state){
            s_motor_high_pin = !s_motor_high_pin;
        }
    }
}