/*

    - Webserver:

    - NixieTime:
        (- get the timezone working correctly...)
        - put sntp instance into NixieTime class
        - make a nixieTime::function to initialize and start sntp service
        - make a nixieTime::function to stop sntp service
        - make a nixieTime::function to set esp time manually. When doing this, set ds3231 time accordingly.
            --> to manualy set time: get it as struct, get current local time, change s, min, hour etc., the try and set with settimeofday
        - if spi slave is not here, then do not synch esp-time with spi slave, rather just synch via sntp and leave time as it is
        - start sntp service in a seperate function so that ios clear what is done (not in the constructor of nixie-time...)

    - create a MainStateMachine.cpp and create a while 1 loop. Create framework for some stuff
        - try to pass wifi, webserver objects to MainStateMachine as pointers (or reference) so that MainStateMachine has access to all objetcs



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
#include "maag_spi_host.h"
#include "maag_spi_device.h"

#include "nixie_projdefs.h"
#include "nixie_webserver.h"
#include "nixie_testClass.h"
#include "nixie_ds3231.h"
#include "nixie_time.h"
#include "nixie_hv5622.h"

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
    wifi.setSSID("FRITZ!Box 7583 AE 2.4 Ghz");
    wifi.setPW("72176317897889201379");
    // maagWifi.init_ap();
    wifi.init_sta();
    wifi.createSTAAutoConnectTask(5000, 0);
    // =====================================================================
    // webserver object
    //
    // NixieWebserver webserver;  // create webserver object
    // webserver.createServer(0); // start webserver --> create freRtos tasks
    //
    // =====================================================================
    // GPIO's --> just testcode
    //
    // GpioInput gpioIn(GPIO_NUM_14, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE);
    // GpioOutput gpioOut(GPIO_NUM_5, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE);
    // gpioOut.setOutput(true);
    // gpio2.setOutput(true);
    //
    // =====================================================================
    // DS3231 RTC (external clock)
    //
    // creat a i2c port
    MaagI2CPort i2c;
    i2c.initPort(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, I2C_MODE_MASTER);
    // create DS3231 i2c device and hook to port. ds3231 is handled in nixieTime class instance
    DS3231 ds3231(i2c.getPort());
    //
    // =====================================================================
    // SNTP (online clock)
    //
    // create sntp instance but don't start it yet. It will be started in nixieTime class instance
    MaagSNTP sntp;
    // set the sntp synch interval here in minutes
    sntp.setSynchInterval(1 * 60 * 1000);
    //
    // =====================================================================
    // NIXIE TIME (main functionality)
    //
    // create nixie time instance and pass sntp & ds3231 objects as reference. Start sntp service
    NixieTime nixieTime(sntp, ds3231);
    // offset to GMT, because timezones arn't working correctly yet
    nixieTime.setLocalTimeOffset(1 * 3600);
    // start a synchronisation task that will try and synch esp-time to ds3231 time
    
    // --> commented out for now so that rtc doesn't mess up time for testing
    // nixieTime.createSynchTask(1, NIXIE_TIME_DS3231_AS_MASTER, 10 * 1000, 1);
    
    //
    /* Notes:
        - use nixieTime.getEspTime(ESP_TIME_LOCAL) for getting time. Although ESP_TIME_GMT gives the same value
        after we se a timezone. I honestly have no idea why they are the same. But who cares. Synchronizing between
        esp and ds3231 is done in GMT time. Somehow it works, do not ask me how.
    */
    //
    // =====================================================================
    // SPI --> writing stuff to nixie tubes
    // 
    // create a spi host with miso, mosi (i.e. data), clk
    MaagSpiHost spi;
    spi.initHost(SPI2_HOST, GPIO_NUM_19, GPIO_NUM_23, GPIO_NUM_18);
    // create an hv5622 spi device
    NixieHv5622 hv5622;
    // init the device and connect to a host. Set clk frequency and "latch" pin. Use spi mode 1 here
    hv5622.initDevice(spi.getHostDevice(), 10000, 1, GPIO_NUM_4);
    // initialize gpios needed for blanking and polarity
    hv5622.initGpios(GPIO_NUM_16, GPIO_NUM_17);



    // TickType_t previousWakeTime = xTaskGetTickCount();
    while (true)
    {     
        // temporarily write time every x seconds here in main. Will be handled in a nixieTime task in future
        hv5622.writeTimeToHv5622(nixieTime.getEspTime(ESP_TIME_LOCAL));
        // log times to console
        nixieTime.logTimes();

        
        ESP_LOGI(TAG, "Main Looping");
        // xTaskDelayUntil(&previousWakeTime,(1000 / portTICK_PERIOD_MS));
        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}
