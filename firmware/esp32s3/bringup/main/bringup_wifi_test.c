#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "regex.h"

#define TAG "WIFI_TEST"

#ifdef CONFIG_WIFI_TEST

#define DEFAULT_SCAN_LIST_SIZE 20

#ifdef CONFIG_EXAMPLE_USE_SCAN_CHANNEL_BITMAP

#define USE_CHANNEL_BITMAP 1
#define CHANNEL_LIST_SIZE 3
static uint8_t channel_list[CHANNEL_LIST_SIZE] = {1, 6, 11};
#endif /*CONFIG_EXAMPLE_USE_SCAN_CHANNEL_BITMAP*/


static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 1000) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } 
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

static void perform_wifi_scan() {
    ESP_LOGI(TAG, "Performing WiFi scan...");

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_scan_start(NULL, true);

    ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", number);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
    for (int i = 0; i < number; i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "\tRSSI \t\t%d", ap_info[i].rssi);
        ESP_LOGI(TAG, "\tChannel \t\t%d", ap_info[i].primary);
    }
}
static void wifi_test_task(void *arg) {
    ESP_LOGI(TAG, "WiFi test started");

#ifdef CONFIG_WIFI_TEST_MODE_CONNECT

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_LOGI(TAG, "WiFi test connect started");
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_TEST_CONNECT_SSID,
            .password = CONFIG_WIFI_TEST_CONNECT_PASSWORD,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "WiFi test connect completed");

#endif

    while (1)
    {
#ifdef CONFIG_WIFI_TEST_MODE_SCAN
        perform_wifi_scan();

        vTaskDelay(pdMS_TO_TICKS(CONFIG_WIFI_TEST_SCAN_DELAY_S * 1000));

#else
        vTaskDelay(pdMS_TO_TICKS(10000));
#endif  
    }
}
#endif


void wifi_test_configure() {

#ifdef CONFIG_WIFI_TEST
    ESP_LOGI(TAG, "WiFi test configuration started");
    
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_LOGI(TAG, "WiFi test configuration completed");
#endif
}

void wifi_test_start() {
#ifdef CONFIG_WIFI_TEST
    ESP_LOGI(TAG, "WiFi test task starting");

    xTaskCreate(wifi_test_task, "wifi_test_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "WiFi test task started");
#endif
}