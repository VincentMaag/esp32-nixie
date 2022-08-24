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

// =====================================================================
// maag_wifi class
class MaagWifi
{
private:
    // static event handler. Has to be static because of how it is called in esp-idf functions
    static esp_err_t event_handler(void *ctx, system_event_t *event);
    // connection status. Must be static because it is passed by static event handler
    static bool m_bConnectionStatus;
	// event handling and connection status
    EventGroupHandle_t m_maagWifi_event_group;
    // string stuff
    std::string m_sIPAdress;
    std::string m_sGWAdress;
    std::string m_sNMAdress;
    std::string m_sDNSAdress;
    std::string m_sSSID;
    std::string m_sPW;
    //
    TickType_t m_autoConnectTasTicksToDelay = 5000;
    // auto connect task that polls sta connection status and tries to connect if disconnected
    static void sta_auto_connect_task(void * pArgs);
public:
	MaagWifi();
    // initialize and start wifi in Access Point (ap) mode
	esp_err_t init_ap();
    // initialize and start wifi in Station (sta) mode. Will try to connect to configured AP
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
    // set DNS
    void setDNS(std::string sDNSAdress_);
    // set ssid
    void setSSID(std::string sSSID_);
    // set password
    void setPW(std::string sPW_);
    // create a connection task who's only job is to try and stay connected if in sta mode
    esp_err_t createSTAAutoConnectTask(TickType_t autoConnectTasTicksToDelay_, BaseType_t xCoreID_);
};

#endif /* __MAAG_WIFI_H__ */