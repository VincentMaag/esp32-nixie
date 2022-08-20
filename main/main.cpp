/*



    - add wifi.activateAutoConnect() --> this will create a task that tries to reconnect if disconnected. This task is a xTaskCreate so that it will run indepandantly

    - create a MainStateMachine.cpp and create a while 1 loop. Create framework for some stuff
        - try to pass wifi, webserver objects to MainStateMachine as pointers (or reference) so that MainStateMachine has access to all objetcs


    + gpio.cpp, gpio.h --> set up some neat functions to get status of gpio's --> and setup functions, try to create a class "gpio"!
        + create base classes for gpio and in/out
        + put gpio stuff into a component

    (- get the RTC functionality of stepper.c bla to work in cpp so that we can write stuff)


    - create an SPI interface Framework
        - class 


    - find out how to turn on optimiziation of compiler and experiment with it


THSI IS MAIN BRANCH



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

#include "maag_wifi.h"
#include "maag_gpio.h"
#include "maag_sntp.h"
#include "maag_i2c_port.h"
#include "maag_i2c_device.h"

#include "nixie_projdefs.h"
#include "nixie_webserver.h"
#include "nixie_testClass.h"
#include "nixie_ds3231.h"


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
    
    // not sure why i added next lines...
    // ESP_LOGI(TAG, "Initializing netif");
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_LOGI(TAG, "Initializing event loop");
    // ESP_ERROR_CHECK( esp_event_loop_create_default() );
    // =====================================================================
    // Wifi object
    MaagWifi wifi;
    wifi.setIP("192.168.178.140");
    wifi.setGW("192.168.178.1");
    wifi.setDNS("8.8.8.8");
    wifi.setSSID("FRITZ!Box 5490 WT");
    wifi.setPW("55940362741817360715");
    // maagWifi.init_ap();
    wifi.init_sta();
    // =====================================================================
    // webserver object
    // MaagWebserver webserver;
    // webserver.createServer(0);

    NixieWebserver webserver;  // create webserver object
    webserver.createServer(0); // start webserver --> create freRtos tasks

    // =====================================================================
    // Testobject
    // MyTestClass myTestClass;
    // xTaskCreatePinnedToCore(myTestClass.freeRtosTask , "test_class", 4096, &myTestClass.arg, 5, NULL, 0);

    // =====================================================================
    // GPIO's
    // EspGpio gpio1(GPIO_NUM_18, GPIO_MODE_INPUT);
    // EspGpio gpioOut(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    // GPIO_MODE_INPUT_OUTPUT

    // EspGpio gpioIn(GPIO_NUM_18, GPIO_MODE_INPUT);

    // EspGpio gpioOut(GPIO_NUM_15, GPIO_MODE_OUTPUT, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE, GPIO_INTR_DISABLE);

    // Gpio gpio_1(GPIO_NUM_13, GPIO_MODE_INPUT_OUTPUT, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE, GPIO_INTR_DISABLE);

    // GpioInput gpioIn(GPIO_NUM_14);

    // GpioOutput gpioOut(GPIO_NUM_15, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE);

    // gpio1.setOutput(true);
    // gpio2.setOutput(true);


    // =====================================================================
    // I2C

    MaagI2CPort i2c;
    i2c.initPort(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, I2C_MODE_MASTER);


    // uint8_t time_addr = 0x00;
    // uint8_t data[7] = {0};


    // MaagI2CDevice ds3231;
    // ds3231.setPort(i2c.getPort());
    // ds3231.setDeviceAddress(0x68);


    // create DS3231 i2c device and set port
    DS3231 ds3231(i2c.getPort());


    // i2c.i2c_master_init(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22)








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
    // SNTP
    MaagSNTP sntp;
    sntp.initStart();
    
    
    // initialize_sntp_maag();

    // =====================================================================
    // Pass all objects as reference (or pointers lol) to main state machine

    // ...
    bool bToggle = false;

    while (true)
    {

        if (wifi.getConnectionStatus() == false)
        {
            ESP_LOGW(TAG, "caught disconnected esp. Waiting just to be sure...");
            // wait a bit if we catch a disconnetced esp
            vTaskDelay((5000 / portTICK_PERIOD_MS));
            // if still disconnected, try and connect
            if (wifi.getConnectionStatus() == false)
            {
                wifi.wifi_try_connect_sta();
            }
        }

        // don't do this because the freeRtos task is a while loop ;)
        // ESP_LOGW(TAG, "CALLING FREERTOS_TASK IN A WHILE LOOP");
        // myTestClass.freeRtosTask(&myTestClass.arg);

        // ESP_LOGW(TAG, "nixie webserver requests: %i, gpioOut: %i, toggle: %i", webserver.getCommunicationCounter(), gpioIn.getInput(), bToggle);
        // ESP_LOGW(TAG, "Rönning");

        // get current system time, set by sntp server
        // time_t now = 0;
        // time(&now);
	    struct tm espTime = {0};
        struct tm ds3231Time = {0};
	    // localtime_r(&now, &espTime);

        // set time of ds3231
        // ds3231.setTime(espTime);
        ds3231.setTimeToEspSystemTime();
        // short pause
        vTaskDelay((100 / portTICK_PERIOD_MS));
        // read time of ds3231
        ds3231Time = ds3231.getTime();


        char strftime_buf_esp[64];
        strftime(strftime_buf_esp, sizeof(strftime_buf_esp), "%c", &espTime);
        char strftime_buf_ds3231[64];
        strftime(strftime_buf_ds3231, sizeof(strftime_buf_ds3231), "%c", &ds3231Time);
        
        ESP_LOGW(TAG, "The current date/times are: === esp: %s === ds3231: %s", strftime_buf_esp, strftime_buf_ds3231);


        // ds3231.setTime(&espTime);

        // ds3231.read(&time_addr, 1, &data[0], 7);

        // ESP_LOGW(TAG, "Time read: %i, %i, %i, %i, %i, %i, %i",data[0],data[1],data[2],data[3],data[4],data[5],data[6]);
        
        // ds3231.convertDisplayTime(data);

        // bToggle = !bToggle;
        // gpioOut.setOutput(bToggle);

        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}
