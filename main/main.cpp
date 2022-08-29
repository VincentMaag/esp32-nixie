/*

    - SPI interface Framework
        - seperate port (rename to host) and device in seperate cpp files
        - host: 
            - pass relevant configuration information --> max transfer size, 
        - device:
            - initDevice: clock_speed_hz
            - spi_device_interface_config_t: understand this and set flags accordingly... --> probably no flags set for normal mode
            - write() --> writePolling() --> pass variable stuff to write using pointers etc. Check length with max size m_max_size
            - check out if we can do a writeInterrupt() --> callback functions? not needed atm
            - get a readPolling() function going, but maybe hard to test atm


    - create a MainStateMachine.cpp and create a while 1 loop. Create framework for some stuff
        - try to pass wifi, webserver objects to MainStateMachine as pointers (or reference) so that MainStateMachine has access to all objetcs

    (- get the RTC functionality of stepper.c bla to work in cpp so that we can write stuff)

    - wifi: use the "this" way to pass the object into a FreeRtos Task. Do this in http_serve task for wifi!


    - find out how to turn on optimiziation of compiler and experiment with it


*/

#include <string.h>
#include <string>

#include <time.h>
#include <sys/time.h>

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

#include "maag_spi_port.h"

#include "nixie_projdefs.h"
#include "nixie_webserver.h"
#include "nixie_testClass.h"
#include "nixie_ds3231.h"
#include "nixie_time.h"


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
    wifi.createSTAAutoConnectTask(5000, 0);
    // =====================================================================
    // webserver object
    // MaagWebserver webserver;
    // webserver.createServer(0);
    NixieWebserver webserver;  // create webserver object
    webserver.createServer(0); // start webserver --> create freRtos tasks
    // =====================================================================
    // GPIO's
    GpioInput gpioIn(GPIO_NUM_14,GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE);
    // GpioOutput gpioOut(GPIO_NUM_15, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE);
    // gpio1.setOutput(true);
    // gpio2.setOutput(true);
    // =====================================================================
    // I2C
    // creat a i2c port
    MaagI2CPort i2c;
    i2c.initPort(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, I2C_MODE_MASTER);
    // =====================================================================
    // DS3231
    // create DS3231 i2c device and set port. ds3231 is handled in nixieTime instance
    DS3231 ds3231(i2c.getPort());
    // =====================================================================
    // SNTP
    // create sntp instance. sntp service must not be started here because it is started in nixieTiem instance
    MaagSNTP sntp;
    // =====================================================================
    // Nixie Time
    // create nixie time instance and pass sntp & ds3231 objects as reference
    NixieTime nixieTime(sntp, ds3231);
    // start a synchronisation task that will try and synch esp-time to ds3231 time
    nixieTime.createSynchTask(2,NIXIE_TIME_DS3231_AS_MASTER,30000,1);



    // =====================================================================
    // SPI
    MaagSpiPort spi;
    spi.initHost(SPI2_HOST,GPIO_NUM_19, GPIO_NUM_23,GPIO_NUM_18);

    MaagSpiDevice mux;
    mux.initDevice(spi.getHostDevice());



    /* SPI	MOSI	MISO	CLK	    CS
    VSPI	GPIO 23	GPIO 19	GPIO 18	GPIO 5
    */





    while (true)
    {

        ESP_LOGE(TAG, "trying to write something on spi bus");
        mux.write(0xB3,0x7C);        

        // ESP_LOGE(TAG, "Main doing shit all :)");
        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}
