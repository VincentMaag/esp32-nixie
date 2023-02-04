/*

    - Webserver:

    - NixieTime:
        - get the timezone working correctly...
        - make a nixieTime::function to stop sntp service
        - make a nixieTime::function to set esp time manually. When doing this, set ds3231 time accordingly.
            --> to manualy set time: get it as struct, get current local time, change s, min, hour etc., the try and set with settimeofday
        - if spi slave is not here, then do not synch esp-time with spi slave, rather just synch via sntp and leave time as it is
        - start sntp service in a seperate function so that its clear what is done (not in the constructor of nixie-time...)

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
    // =====================================================================
    // Wifi object
    MaagWifi wifi;
    wifi.setIP("192.168.178.140");
    wifi.setGW("192.168.178.1");
    wifi.setDNS("8.8.8.8");
    wifi.setSSID("FRITZ!Box 7583 AE 2.4 Ghz");
    wifi.setPW("72176317897889201379");
    // wifi.setSSID("Nixie AP");
    // wifi.setPW("ScheissEgalEh");
    //wifi.init_ap();
    wifi.init_sta();
    wifi.createSTAAutoConnectTask(5000, 0);
    // =====================================================================
    // DS3231 (RTC - external clock)
    //
    // creat a i2c port
    MaagI2CPort i2c;
    i2c.initPort(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, I2C_MODE_MASTER);
    // create DS3231 i2c device and hook to port. ds3231 is handled in nixieTime class instance
    DS3231 ds3231(i2c.getPort());
    // =====================================================================
    // SNTP (online clock)
    //
    MaagSNTP sntp;
    sntp.setSynchInterval(1 * 60 * 1000);
    // initialize sntp with our custom static callback and start sntp service. The callbalck func is static,
    // so we are allowed to set it here, even before we create an instance of nixieTime
	sntp.setSyncNotificationCb(NixieTime::nixieTimeSNTPSyncNotificationCb);
	sntp.initStart();
    // =====================================================================
    // NIXIE TIME (main functionality)
    //
    // create our synch-object that will synchronize esp-, sntp- and rtc-times
    NixieTime nixieTime(ds3231);
    // start a synchronisation task that will try and synch esp-time to ds3231 time
    //nixieTime.createSynchTask(1, NIXIE_TIME_DS3231_AS_MASTER, 5 * 1000, 1);
    nixieTime.createSynchTask(1, NIXIE_TIME_ESP_AS_MASTER, 5 * 1000, 1);
    // =====================================================================
    // Webserver
    //
    NixieWebserver webserver;  // create webserver object
    webserver.createServer(0); // start webserver --> create freRtos tasks    
    webserver.passWebseverParams(&nixieTime);
    // =====================================================================
    // SPI ( HV5622 - writing stuff to nixie tubes)
    // 
    MaagSpiHost spi;
    spi.initHost(SPI2_HOST, GPIO_NUM_19, GPIO_NUM_23, GPIO_NUM_18);
    // spi-device for time (hh:mm:ss)
    NixieHv5622 hv5622_time;
    hv5622_time.initDevice(spi.getHostDevice(), 10000, 1, GPIO_NUM_4);
    hv5622_time.initGpios(GPIO_NUM_16, GPIO_NUM_17);
    // second spi device for date (dd:mm:yy)
    NixieHv5622 hv5622_date;
    hv5622_date.initDevice(spi.getHostDevice(), 10000, 1, GPIO_NUM_32);
    hv5622_date.initGpios(GPIO_NUM_25, GPIO_NUM_26);
    

    // TickType_t previousWakeTime = xTaskGetTickCount();
    while (true)
    {     

        
        // temporarily write time every x seconds here in main. Will be handled in a nixieTime task in future
        //hv5622.writeTimeToHv5622(nixieTime.getEspTime(ESP_TIME_LOCAL));
        hv5622_time.writeTimeToHv5622(nixieTime.getEspTime(ESP_TIME_LOCAL));
        hv5622_date.writeDateToHv5622(nixieTime.getEspTime(ESP_TIME_LOCAL));
        
        // log times to console
        nixieTime.logTimes();
        
        
        //ESP_LOGI(TAG, "Main Looping");
        // xTaskDelayUntil(&previousWakeTime,(1000 / portTICK_PERIOD_MS));
        vTaskDelay((1000 / portTICK_PERIOD_MS));
    }

    ESP_LOGI(TAG, "FINISHED MAIN");
}
