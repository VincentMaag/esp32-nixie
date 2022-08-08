/*
    

    - cleanup all cpp files etc. with comments etc.

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

#include "nixie_testClass.h"


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
    // webserver.createServer(0);

    NixieWebserver webserver;      // create webserver object
    webserver.createServer(0);     // start webserver --> create freRtos tasks


    // =====================================================================
    // Testobject
    //MyTestClass myTestClass;
    //xTaskCreatePinnedToCore(myTestClass.freeRtosTask , "test_class", 4096, &myTestClass.arg, 5, NULL, 0);
    



    /* we want these tasks for nixie:
        - wifi, these tasks are created via object (not exactly sure where though...). We can restart stuff via the object
        - webserver, these tasks are created via object, server defined in http_server_task
        - I2C, this task will be created in the object. Handling in/out-data will be done in the task. We can use get/set functions of the object
        - flash memory: we will create a class to handle memory stuff more elegantly! no need for a task, this will be method-driven only
        - MainStateMachine that will handle all the objects etc. I want to create a MainStateMachine-Class that creates all the tasks via the objects.
            --> we will see if this works. Try to create a MainClass with "class-members" (wifi, webserver etc). See if we can get all to work. Else
            we will have to create the objects here in main and then pass some references.



    */


    // =====================================================================
    // Pass all objects as reference (or pointers lol) to main state machine
    

    
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

        // don't do this because the freeRtos task is a while loop ;)
        // ESP_LOGW(TAG, "CALLING FREERTOS_TASK IN A WHILE LOOP");
        // myTestClass.freeRtosTask(&myTestClass.arg);



        ESP_LOGW(TAG, "current nixie webserver requests: %i",webserver.getCommunicationCounter());
        // ESP_LOGW(TAG, "current nixie webserver requests: %i",webserver.);

        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}