/*
    ...

*/
#ifndef __MAAG_WIFI_H__
#define __MAAG_WIFI_H__

#include <string>

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"

//using namespace std;

// =====================================================================
// maag_wifi class
class MaagWifi
{
private:
	// event handling and connection status
    EventGroupHandle_t maagWifi_event_group;
    // event handler. must be static bcause. Don't understand it yet but i guess it will become clearer in future :)
    static esp_err_t event_handler(void *ctx, system_event_t *event);
    // connection status. Must be static because it is passed by static event handler
    static bool bConnectionStatus;
    // string stuff
    std::string sIPAdress;
    std::string sGWAdress;
    std::string sNMAdress;
    std::string sSSID;
    std::string sPW;
public:
	MaagWifi(/* args */);
	~MaagWifi();
    // initialize and start wifi in Access Point (ap) mode
	esp_err_t init_ap();
    // initialize and start wifi in Station (sta) mode
	esp_err_t init_sta();
    // Stop wifi
	esp_err_t stop_wifi();
    // Start wifi
	esp_err_t start_wifi();
    // try and reconnect if in sta mode
    void wifi_try_connect_sta();
    // get connection status
    bool getConnectionStatus();
    // set ip adress
    void setIP(std::string sIPAdress_);
    // set gateway.
    void setGW(std::string sGWAdress_);
    // set netmask
    void setNM(std::string sNMAdress_);
    // set ssid
    void setSSID(std::string sSSID_);
    // set password
    void setPW(std::string sPW_);
};


void maag_wifi_task(void* arg);


#endif /* __MAAG_WIFI_H__ */