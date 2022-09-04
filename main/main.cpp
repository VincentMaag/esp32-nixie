/*

    - Webserver:

    - NixieTime:
        (- get the timezone working correctly...)
        - put sntp instance into NixieTime class
        - make a nixieTime::function to initialize and start sntp service
        - make a nixieTime::function to stop sntp service
        - make a nixieTime::function to set esp time manually. When doing this, set ds3231 time accordingly.
            --> to manualy set time: get it as struct, get current local time, change s, min, hour etc., the try and set with settimeofday



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
    wifi.setSSID("FRITZ!Box 5490 WT");
    wifi.setPW("55940362741817360715");
    // maagWifi.init_ap();
    wifi.init_sta();
    wifi.createSTAAutoConnectTask(5000, 0);
    // =====================================================================
    // webserver object
    NixieWebserver webserver;  // create webserver object
    webserver.createServer(0); // start webserver --> create freRtos tasks
    // =====================================================================
    // GPIO's --> just testcode
    // GpioInput gpioIn(GPIO_NUM_14, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE);
    // GpioOutput gpioOut(GPIO_NUM_5, GPIO_PULLDOWN_ENABLE, GPIO_PULLUP_DISABLE);
    // gpioOut.setOutput(true);
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
    // create sntp instance but don't start it yet
    MaagSNTP sntp;
    // set the sntp synch interval here in minutes
    sntp.setSynchInterval(5 * 60 * 1000);
    // =====================================================================
    // NIXIE TIME
    // create nixie time instance and pass sntp & ds3231 objects as reference. Start sntp service
    NixieTime nixieTime(sntp, ds3231);
    // some configurations
    nixieTime.setLocalTimeOffset(2*3600);
    // start a synchronisation task that will try and synch esp-time to ds3231 time, in seconds
    nixieTime.createSynchTask(2, NIXIE_TIME_DS3231_AS_MASTER, 30 * 1000, 1);
    /* Notes:
        - use nixieTime.getEspTime(ESP_TIME_LOCAL) for getting time. Although ESP_TIME_GMT gives the same value
        after we se a timezone. I honestly have no idea why they are the same. But who cares. Synchronizing between
        esp and ds3231 is done in GMT time. Somehow it works, do not ask me how.

    */
    // =====================================================================
    // SPI
    // create a spi host with miso, mosi, clk
    MaagSpiHost spi;
    spi.initHost(SPI2_HOST, GPIO_NUM_19, GPIO_NUM_23, GPIO_NUM_18);
    // create an hv5622 spi device
    NixieHv5622 hv5622;
    // init the device and connect to a host. Set clk frequency
    hv5622.initDevice(spi.getHostDevice(), 10000, GPIO_NUM_5);
    // initialize gpios needed for hv5622 coomunication
    hv5622.initGpios(GPIO_NUM_20, GPIO_NUM_21);






    // TickType_t previousWakeTime = xTaskGetTickCount();
    while (true)
    {
        
        hv5622.writeTimeToHv5622(nixieTime.getEspTime(ESP_TIME_LOCAL));
        // ESP_LOGI(TAG, "Setting esp to 0 and syncronizing ds3231");
        
        // struct tm tm_ = {};
        // struct timeval now = {};
		// // refactor tm into time_t, set seconds of timeval
		// now.tv_sec = mktime(&tm_);
		// // set esp32 system time
		// settimeofday(&now, NULL);
        // nixieTime.synchTime(NIXIE_TIME_ESP_AS_MASTER);

        ESP_LOGE(TAG, "Main Looping");
        // xTaskDelayUntil(&previousWakeTime,(pMaagWifi->m_autoConnectTasTicksToDelay / portTICK_PERIOD_MS));
        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}
