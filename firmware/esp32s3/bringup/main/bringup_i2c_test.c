#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/i2c_master.h>
#include <esp_log.h>

#define TAG "I2C_TEST"

#ifdef CONFIG_I2C_TEST

#define I2C_PORT 0
#define I2C_TIMEOUT_VALUE_MS 50

i2c_master_bus_handle_t i2c_bus_handle;

static void perform_i2c_scan() {
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            // fflush(stdout);
            address = i + j;
            esp_err_t ret = i2c_master_probe(i2c_bus_handle, address, I2C_TIMEOUT_VALUE_MS);

            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
        // fflush(stdout);
    }
}

static void i2c_test_task(void *arg) {
    ESP_LOGI(TAG, "I2C test started");

    while (1)
    {
        perform_i2c_scan();
        
        vTaskDelay(pdMS_TO_TICKS(CONFIG_I2C_RESCAN_DELAY_S * 1000));
    }
}
#endif

void i2c_test_configure() {
#ifdef CONFIG_I2C_TEST
    ESP_LOGI(TAG, "I2C configuration started");

    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = CONFIG_I2C_SCL_PIN,
        .sda_io_num = CONFIG_I2C_SDA_PIN,
        .glitch_ignore_cnt = 7,
#ifdef CONFIG_I2C_ENABLE_PULLUP
        .flags.enable_internal_pullup = true,
#else
        .flags.enable_internal_pullup = false,
#endif
    };

    ESP_LOGI(TAG, "I2C: SDA %d SCL %d", CONFIG_I2C_SDA_PIN, CONFIG_I2C_SCL_PIN);

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));

    ESP_LOGI(TAG, "I2C configuration completed");
#endif
}



void i2c_test_start() {
#ifdef CONFIG_I2C_TEST
    ESP_LOGI(TAG, "I2C test task starting");

    xTaskCreate(i2c_test_task, "i2c_test_task", 2048, NULL, 10, NULL);

#endif
}
