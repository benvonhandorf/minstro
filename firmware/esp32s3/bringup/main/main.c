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
#include "esp_wifi.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "bringup_led_test.h"
#include "bringup_i2c_test.h"
#include "bringup_wifi_test.h"
#include "bringup_gpio_test.h"

static const char *TAG = "bring_up";

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Configuring tests");

    led_test_configure();

    i2c_test_configure();

    gpio_test_configure();

    wifi_test_configure();

    ESP_LOGI(TAG, "Starting tests");

    led_test_start();

    i2c_test_start();

    gpio_test_start();

    wifi_test_start();

    ESP_LOGI(TAG, "Tests started");

    while (1) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
