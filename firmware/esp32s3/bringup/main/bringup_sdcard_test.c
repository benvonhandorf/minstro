#include "bringup_sdcard_test.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define TAG "sdcard"

#ifdef CONFIG_SDCARD_TEST

#define BOARD_MINSTRO 1
// #define BOARD_CARDPUTER   1
// #define BOARD_XIAO   1

//Force mode for easier board switching
#define CONFIG_SDCARD_TEST_MODE_SPI 1
#undef CONFIG_SDCARD_TEST_MODE_SDMMC
//or
// #undef CONFIG_SDCARD_TEST_MODE_SPI
// #define CONFIG_SDCARD_TEST_MODE_SDMMC    1


// Known Boards

#ifdef BOARD_MINSTRO
// Minstro Pins
// GPIO     SDMMC   SPI
// 38       DAT1
// 39       DAT0    MISO
// --       Vss
// 40       CLK     SCLK
// --       Vdd
// 41       CMD     MOSI
// 42       DAT3    CS
// 43       DAT2
// --
// 44       DET     DET

#ifdef CONFIG_SDCARD_TEST_MODE_SPI
//SPI Mode
#define CONFIG_SDCARD_TEST_MISO_PIN     GPIO_NUM_39
#define CONFIG_SDCARD_TEST_MOSI_PIN     GPIO_NUM_41  
#define CONFIG_SDCARD_TEST_SCK_PIN      GPIO_NUM_40
#define CONFIG_SDCARD_TEST_CS_PIN       GPIO_NUM_42
#define CONFIG_SDCARD_TEST_CD_PIN       GPIO_NUM_44

#endif

#ifdef CONFIG_SDCARD_TEST_MODE_SDMMC

//SDMMC Mode
#define CONFIG_SDCARD_TEST_DAT0_PIN     GPIO_NUM_39
#define CONFIG_SDCARD_TEST_CMD_PIN     GPIO_NUM_41
#define CONFIG_SDCARD_TEST_SCK_PIN      GPIO_NUM_40
#define CONFIG_SDCARD_TEST_DAT1_PIN       GPIO_NUM_38
#define CONFIG_SDCARD_TEST_DAT2_PIN       GPIO_NUM_43
#define CONFIG_SDCARD_TEST_DAT3_PIN       GPIO_NUM_42
#define CONFIG_SDCARD_TEST_CD_PIN        GPIO_NUM_44

//Force 1 bit mode
#define CONFIG_SDCARD_TEST_DAT1_PIN       -1
#define CONFIG_SDCARD_TEST_DAT2_PIN       -1
#define CONFIG_SDCARD_TEST_DAT3_PIN       -1


#endif

#endif


#ifdef BOARD_XIAO
//Xiao Sense pins

#ifdef CONFIG_SDCARD_TEST_MODE_SPI

//SPI Mode
#define CONFIG_SDCARD_TEST_MISO_PIN     GPIO_NUM_8
#define CONFIG_SDCARD_TEST_MOSI_PIN     GPIO_NUM_9  
#define CONFIG_SDCARD_TEST_SCK_PIN      GPIO_NUM_7
#define CONFIG_SDCARD_TEST_CS_PIN       GPIO_NUM_3
#define CONFIG_SDCARD_TEST_CD_PIN -1

#endif

#ifdef CONFIG_SDCARD_TEST_MODE_SDMMC

//SDMMC Mode
#define CONFIG_SDCARD_TEST_DAT0_PIN     GPIO_NUM_8
#define CONFIG_SDCARD_TEST_CMD_PIN     GPIO_NUM_9  
#define CONFIG_SDCARD_TEST_SCK_PIN      GPIO_NUM_7
#define CONFIG_SDCARD_TEST_DAT1_PIN       -1
#define CONFIG_SDCARD_TEST_DAT2_PIN       -1
#define CONFIG_SDCARD_TEST_DAT3_PIN       -1
#define CONFIG_SDCARD_TEST_CD_PIN -1

#endif


#endif

#ifdef BOARD_CARDPUTER

//M5 Cardputer

#ifdef CONFIG_SDCARD_TEST_MODE_SPI
//SPI Mode
#define CONFIG_SDCARD_TEST_MISO_PIN     GPIO_NUM_39
#define CONFIG_SDCARD_TEST_MOSI_PIN     GPIO_NUM_14 
#define CONFIG_SDCARD_TEST_SCK_PIN      GPIO_NUM_40
#define CONFIG_SDCARD_TEST_CS_PIN       GPIO_NUM_12
#define CONFIG_SDCARD_TEST_CD_PIN -1

#endif

#ifdef CONFIG_SDCARD_TEST_MODE_SDMMC
//SDMMC Mode
#define CONFIG_SDCARD_TEST_DAT0_PIN     GPIO_NUM_39
#define CONFIG_SDCARD_TEST_CMD_PIN     GPIO_NUM_14 
#define CONFIG_SDCARD_TEST_SCK_PIN      GPIO_NUM_40
#define CONFIG_SDCARD_TEST_DAT1_PIN       -1
#define CONFIG_SDCARD_TEST_DAT2_PIN       -1
#define CONFIG_SDCARD_TEST_DAT3_PIN       -1
#define CONFIG_SDCARD_TEST_CD_PIN -1

#endif

#endif

// Mode specific pin setups
//Shared
#define PIN_NUM_CD CONFIG_SDCARD_TEST_CD_PIN   // Card detect
#define PIN_NUM_CLK CONFIG_SDCARD_TEST_SCK_PIN // SCK

#ifdef CONFIG_SDCARD_TEST_MODE_SPI
// SPI Pin Names
#define PIN_NUM_MISO CONFIG_SDCARD_TEST_MISO_PIN
#define PIN_NUM_MOSI CONFIG_SDCARD_TEST_MOSI_PIN
#define PIN_NUM_CS CONFIG_SDCARD_TEST_CS_PIN

#endif

#ifdef CONFIG_SDCARD_TEST_MODE_SDMMC
// SDMMC Pin Names
#define PIN_NUM_DAT0 CONFIG_SDCARD_TEST_DAT0_PIN
#define PIN_NUM_DAT1 CONFIG_SDCARD_TEST_DAT1_PIN
#define PIN_NUM_CMD CONFIG_SDCARD_TEST_CMD_PIN
#define PIN_NUM_DAT2 CONFIG_SDCARD_TEST_DAT2_PIN
#define PIN_NUM_DAT3 CONFIG_SDCARD_TEST_DAT3_PIN

#endif

#define MOUNT_POINT "/sdcard"
#define TEST_FILE_PATH MOUNT_POINT "/hello.txt"

// Force a simple 50ms toggle on SPI pins
// #define CONFIG_SDCARD_TEST_TOGGLE_GPIO 1

static sdmmc_card_t *card = NULL;
static bool card_mounted = false;

static void list_files(const char *path)
{
    ESP_LOGI(TAG, "Listing files in %s:", path);

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        ESP_LOGE(TAG, "Failed to open directory %s", path);
        return;
    }

    struct dirent *entry;
    int file_count = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        file_count++;
        ESP_LOGI(TAG, "  %s", entry->d_name);
    }

    if (file_count == 0)
    {
        ESP_LOGI(TAG, "  (no files found)");
    }

    closedir(dir);
}

void jtag_uart_pins_to_gpio()
{
    gpio_reset_pin(39);
    gpio_reset_pin(40);
    gpio_reset_pin(41);
    gpio_reset_pin(42);
    gpio_reset_pin(43);
    gpio_reset_pin(44);
}

#ifdef CONFIG_SDCARD_TEST_MODE_SDMMC

static esp_err_t configure_sdcard_infrastructure()
{
    esp_err_t ret;

    ret = sdmmc_host_init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize sdmmc host (%s).", esp_err_to_name(ret));

        return ret;
    }

    return ESP_OK;
}

static esp_err_t mount_sdcard()
{
    esp_err_t ret;

    // Initialize the host
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_1;

    // Configure slot
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
#if PIN_NUM_DAT1 != -1
    slot_config.width = 4; // Use 4-bit mode
#else
    slot_config.width = 1;

#endif
#ifdef PIN_NUM_CD
    slot_config.cd = SDMMC_SLOT_NO_CD; // PIN_NUM_CD;
#else
    slot_config.cd = SDMMC_SLOT_NO_CD;
#endif
    slot_config.wp = SDMMC_SLOT_NO_WP;

    // Configure GPIO pins
    slot_config.clk = PIN_NUM_CLK;
    slot_config.cmd = PIN_NUM_CMD;
    slot_config.d0 = PIN_NUM_DAT0;
    slot_config.d1 = PIN_NUM_DAT1;
    slot_config.d2 = PIN_NUM_DAT2;
    slot_config.d3 = PIN_NUM_DAT3;

    // Mount configuration
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(TAG, "Mounting SD card...");
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. If you want to format the card, set format_if_mount_failed = true.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). Check SD card connections.", esp_err_to_name(ret));
        }
        return ret;
    }

    card_mounted = true;
    ESP_LOGI(TAG, "SD card mounted successfully");

    // Print card info
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

#endif

#ifdef CONFIG_SDCARD_TEST_MODE_SPI

static sdspi_dev_handle_t sdspi_dev_handle;

static esp_err_t configure_sdcard_infrastructure()
{
    esp_err_t ret;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }

    // Initialize the host - Currently (5.4.2) a no-op.
    ret = sdspi_host_init();

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize the sdspi (%s).", esp_err_to_name(ret));

        return ret;
    }

    sdspi_device_config_t dev_config = SDSPI_DEVICE_CONFIG_DEFAULT();

    // dev_config.gpio_cd = PIN_NUM_CD;
    dev_config.gpio_cs = PIN_NUM_CS;
    dev_config.gpio_wp = SDSPI_SLOT_NO_WP;
    dev_config.gpio_int = SDSPI_SLOT_NO_INT;

    ret = sdspi_host_init_device(&dev_config, &sdspi_dev_handle);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize the sdspi host (%s).", esp_err_to_name(ret));

        return ret;
    }

    ESP_LOGI(TAG, "SPI infrastructure configured");

    return ESP_OK;
}

static esp_err_t mount_sdcard()
{
    esp_err_t ret;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = sdspi_dev_handle;

    // Configure slot
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = sdspi_dev_handle;
    slot_config.gpio_cs = PIN_NUM_CS;

    // Mount configuration
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(TAG, "Mounting SD card...");
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. If you want to format the card, set format_if_mount_failed = true.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). Check SD card connections.", esp_err_to_name(ret));
        }
        return ret;
    }

    card_mounted = true;
    ESP_LOGI(TAG, "SD card mounted successfully");

    // Print card info
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

#endif

static void unmount_sdcard()
{
    if (card_mounted)
    {
        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, card);
        card_mounted = false;
        ESP_LOGI(TAG, "SD card unmounted");
    }
}

static esp_err_t create_test_file()
{
    ESP_LOGI(TAG, "Creating test file: %s", TEST_FILE_PATH);

    FILE *f = fopen(TEST_FILE_PATH, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }

    fprintf(f, "Hello, world");
    fclose(f);

    ESP_LOGI(TAG, "Test file created successfully");
    return ESP_OK;
}

static bool check_card_present()
{
#ifdef CONFIG_SDCARD_TEST_MODE_SPI
    ESP_LOGI(TAG, "SPI Mode Pins CLK: %d, MOSI: %d, MISO: %d, CS: %d",
             PIN_NUM_CLK, PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CS);
#endif

#ifdef CONFIG_SDCARD_TEST_MODE_SDMMC
    ESP_LOGI(TAG, "SDMMC Mode Pins CLK: %d, CMD: %d D0: %d, D1: %d, D2: %d, D3: %d",
             PIN_NUM_CLK, PIN_NUM_CMD, PIN_NUM_DAT0, PIN_NUM_DAT1, PIN_NUM_DAT2, PIN_NUM_DAT3);
#endif

#if CONFIG_SDCARD_TEST_CD_PIN != -1
    // Card detect is typically active low
    int cd_level = gpio_get_level(PIN_NUM_CD);
    bool card_present = (cd_level == 0);

    ESP_LOGI(TAG, "Card detect pin (GPIO%d) level: %d, Card present: %s",
             PIN_NUM_CD, cd_level, card_present ? "YES" : "NO");

    return card_present;
#else
    ESP_LOGI(TAG, "No CD pin");

    return true;
#endif
}

static void sdcard_test_task(void *args)
{
    ESP_LOGI(TAG, "SD Card test started");

    uint8_t level = 1;

    while (1)
    {

#ifdef CONFIG_SDCARD_TEST_TOGGLE_GPIO
        level = !level;

#ifdef CONFIG_SDCARD_TEST_MODE_SPI
        gpio_set_level(PIN_NUM_MOSI, level);
        gpio_set_level(PIN_NUM_MISO, level);
        gpio_set_level(PIN_NUM_CLK, level);
        gpio_set_level(PIN_NUM_CD, level);
        gpio_set_level(PIN_NUM_CS, level);

        vTaskDelay(pdMS_TO_TICKS(100));
#else
        gpio_set_level(PIN_NUM_DAT0, level);
#if PIN_NUM_DAT1 > -1
        gpio_set_level(PIN_NUM_DAT1, level);
        gpio_set_level(PIN_NUM_DAT2, level);
        gpio_set_level(PIN_NUM_DAT3, level);
#endif
        gpio_set_level(PIN_NUM_CLK, level);
        gpio_set_level(PIN_NUM_CMD, level);

        vTaskDelay(pdMS_TO_TICKS(100));
#endif

        continue;
#endif

        // Check if card is present
        if (!check_card_present())
        {
            ESP_LOGI(TAG, "No SD card detected");
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }

        // Try to mount the card
        if (mount_sdcard() != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to mount SD card, retrying in 10 seconds...");
            vTaskDelay(pdMS_TO_TICKS(10000));

            continue;
        }

        // List files before creating test file
        ESP_LOGI(TAG, "Files on SD card (before test):");
        list_files(MOUNT_POINT);

        // Create test file
        if (create_test_file() == ESP_OK)
        {
            // List files after creating test file
            ESP_LOGI(TAG, "Files on SD card (after creating test file):");
            list_files(MOUNT_POINT);
        }

        // Unmount and remount to test reliability
        unmount_sdcard();
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Keep card mounted and check periodically
        ESP_LOGI(TAG, "SD card test completed. Will check again in 30 seconds...");
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

#endif // CONFIG_SDCARD_TEST

void sdcard_test_configure()
{
#ifdef CONFIG_SDCARD_TEST

    jtag_uart_pins_to_gpio();

    configure_sdcard_infrastructure();

#if CONFIG_SDCARD_TEST_CD_PIN != -1
    ESP_LOGI(TAG, "Configuring SD card test");

    {
        // Configure card detect pin
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << PIN_NUM_CD),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};

        gpio_config(&io_conf);
    }

#endif

#ifdef CONFIG_SDCARD_TEST_TOGGLE_GPIO

#ifdef CONFIG_SDCARD_TEST_MODE_SPI
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_NUM_MOSI) |
                        (1ULL << PIN_NUM_MISO) |
                        (1ULL << PIN_NUM_CLK) |
                        (1ULL << PIN_NUM_CD) |
                        (1ULL << PIN_NUM_CS),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&io_conf);

    uint8_t level = 1;

    gpio_set_level(PIN_NUM_MOSI, level);
    gpio_set_level(PIN_NUM_MISO, level);
    gpio_set_level(PIN_NUM_CLK, level);
    gpio_set_level(PIN_NUM_CD, level);
    gpio_set_level(PIN_NUM_CS, level);
#else
gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_NUM_DAT0) |
#if PIN_NUM_D1 > -1
                        (1ULL << PIN_NUM_DAT1) |
                        (1ULL << PIN_NUM_DAT2) |
                        (1ULL << PIN_NUM_DAT3) |
#endif
                        (1ULL << PIN_NUM_CLK) |
                        (1ULL << PIN_NUM_CD) |
                        (1ULL << PIN_NUM_CMD),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    gpio_config(&io_conf);

    uint8_t level = 1;

    gpio_set_level(PIN_NUM_DAT0, level);
#if PIN_NUM_D1 > -1
    gpio_set_level(PIN_NUM_DAT1, level);
    gpio_set_level(PIN_NUM_DAT2, level);
    gpio_set_level(PIN_NUM_DAT3, level);
#endif
    gpio_set_level(PIN_NUM_CLK, level);
    gpio_set_level(PIN_NUM_CMD, level);

    vTaskDelay(pdMS_TO_TICKS(100));
#endif

#endif

#endif
}

void sdcard_test_start()
{
#ifdef CONFIG_SDCARD_TEST
    ESP_LOGI(TAG, "Starting SD card test task");

    xTaskCreate(sdcard_test_task, "sdcard_test", 4096, NULL, 5, NULL);
#endif
}