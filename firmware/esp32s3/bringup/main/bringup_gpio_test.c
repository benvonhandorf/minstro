#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/i2c.h>
#include <esp_log.h>

#define TAG "GPIO_TEST"

void gpio_test_configure() {
#ifdef CONFIG_GPIO_TEST
    ESP_LOGI(TAG, "GPIO test configuration started");

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_2);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "GPIO test configuration completed");
#endif
}

void gpio_test_start() {
#ifdef CONFIG_GPIO_TEST
    ESP_LOGI(TAG, "GPIO test configuration started");

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_2);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "GPIO test configuration completed");

#endif
}