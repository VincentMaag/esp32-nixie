
#include <string.h>
#include <string>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "nixie_projdefs.h"
#include "maag_wifi.h"

static const char *TAG = "main";

extern "C" void app_main()
{
    ESP_LOGI(TAG, "STARTING MAIN");
    // =====================================================================
    // global initializing, objects, parameters, nvs, etc.
    //
    ESP_LOGI(TAG, "Initializing nvs");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //
    MaagWifi maagWifi;

    maagWifi.setIP("192.168.178.140");
    maagWifi.setGW("192.168.178.1");
    maagWifi.setSSID("FRITZ!Box 5490 WT");
    maagWifi.setPW("55940362741817360715");

    // maagWifi.init_ap();
    maagWifi.init_sta();

    // =====================================================================
    // create all user tasks
    ESP_LOGI(TAG, "Creating some Tasks");
    // wifi
    //xTaskCreatePinnedToCore(maag_wifi_task, "maag_wifi_task", 4096, (void *)0, 5, NULL, 0);
    // ...


    while (true)
    {

        if (maagWifi.getConnectionStatus() == false)
        {
            ESP_LOGE(TAG, "caught disconnected esp");
            maagWifi.wifi_try_connect_sta();
        }

        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}