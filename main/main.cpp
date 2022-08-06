/*
    
    - create components folder, put all maag_xyz stuff in there. don't forget to correct CMakeLists!
    - cleanup all cpp files etc. with comments etc.
    - add cpu core number to wifi (not sure) and webserver stuff
    - add wifi.activateAutoConnect() --> this will create a task that tries to reconnect if disconnected

    - create a MainStateMachine.cpp and create a while 1 loop. Create framework for some stuff
        - try to pass wifi, webserver objects to MainStateMachine as pointers (or reference) so that MainStateMachine has access to all objetcs

    (- create framework for I2C)

    - see how to get time via http request. Do this in maag_wifi.cpp and try to use an example

    - gpio.cpp, gpio.h --> set up some neat functions to get status of gpio's --> and setup functions, try to create a class "gpio"!

    - get the RTC functionality of stepper.c bla to work in cpp so that we can write stuff








*/






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
// #include "maag_webserver.h"
#include "nixie_webserver.h"


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
    // =====================================================================
    // Wifi object
    MaagWifi wifi;
    wifi.setIP("192.168.178.140");
    wifi.setGW("192.168.178.1");
    wifi.setSSID("FRITZ!Box 5490 WT");
    wifi.setPW("55940362741817360715");
    // maagWifi.init_ap();
    wifi.init_sta();
    // =====================================================================
    // webserver object
    // MaagWebserver webserver;
    // webserver.createServer();

    NixieWebserver webserver;
    webserver.createServer();

    //webserver.http_serve();
    
    // webserver.

    // =====================================================================
    // create all user tasks
    //ESP_LOGI(TAG, "Creating Webserver Test-Task");
    // wifi
    //xTaskCreatePinnedToCore(maag_webserver_task, "maag_webserver_task", 4096, (void *)0, 5, NULL, 0);
    // ...


    while (true)
    {

        if (wifi.getConnectionStatus() == false)
        {
            ESP_LOGW(TAG, "caught disconnected esp. Waiting just to be sure...");
            // wait a bit if we catch a disconnetced esp
            vTaskDelay((5000 / portTICK_PERIOD_MS));
            // if still disconnected, try and connect
            if(wifi.getConnectionStatus() == false){
                wifi.wifi_try_connect_sta();
            }
        }


        // ESP_LOGW(TAG, "current webserver requests: %i",webserver.getCommunicationCounter());

        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}