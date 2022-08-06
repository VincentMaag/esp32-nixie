/*
	This should be a cpp implementation of wifi connection. We will start and:

	1. Create a nonsense task, just to able to create a wifi task in main
	(1.1) play around and try and create a task with a member function of a class. although this might be not very nesessary.
	2. create a wifi class, decide on a good name (start with wifi and hope it works) --> use maag_wifi as we also use maag_lib etc.
		- try and get wifi working with our wifi class (init, ap, sta, etc.)
		- work on quality of life functions (setIP etc., connect, reconnect, so on)


*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "freertos/queue.h"
// #include "esp_system.h"

#include "lwip/ip4_addr.h"
#include "tcpip_adapter.h"

#include "lwip/sys.h"
#include "lwip/api.h"
// #include "http_parser.h"

#include "maag_wifi.h"

// toDo: this goes into class
static const char *TAG = "maag_wifi";

// toDo: maybe this is elegant if it stays here
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN CONFIG_MAX_STA_CONN

// static variables. We must instanciate them here because of linking!
bool MaagWifi::bConnectionStatus = false;

// =====================================================================
// maag_wifi class

MaagWifi::MaagWifi(/* args */)
{

	maagWifi_event_group = xEventGroupCreate();

	bConnectionStatus = false;

	// set default values
	sIPAdress = "192.168.0.1";
	sGWAdress = "192.168.0.1";
	sNMAdress = "255.255.255.0";
	sSSID = "DEFAULT_ESP32";
	sPW = "mypassword";

	ESP_LOGI(TAG, "MaagWifi instance created");
}

MaagWifi::~MaagWifi()
{
	ESP_LOGE(TAG, "MaagWifi instance destroid");
}

esp_err_t MaagWifi::event_handler(void *ctx, system_event_t *event)
{
	switch (event->event_id)
	{
	case SYSTEM_EVENT_AP_STACONNECTED:
		ESP_LOGI(TAG, "station:" MACSTR " join, AID=%d",
				 MAC2STR(event->event_info.sta_connected.mac),
				 event->event_info.sta_connected.aid);
		ESP_LOGI(TAG, "AP STA CONNECTED");
		bConnectionStatus = true;
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		ESP_LOGI(TAG, "station:" MACSTR "leave, AID=%d",
				 MAC2STR(event->event_info.sta_disconnected.mac),
				 event->event_info.sta_disconnected.aid);
		ESP_LOGI(TAG, "AP STA DISCONNECTED");
		bConnectionStatus = false;
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		ESP_LOGI(TAG, "STA CONNECTED");
		bConnectionStatus = true;
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		ESP_LOGI(TAG, "STA DISCONNECTED");
		bConnectionStatus = false;
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
		break;

	default:
		break;
	}
	return ESP_OK;
}

esp_err_t MaagWifi::init_ap()
{
	ESP_LOGI(TAG, "initializing in ap mode");
	// create event group (not sure if needed actually...)
	// maagWifi_event_group = xEventGroupCreate();
	// wifi power
	esp_wifi_set_max_tx_power((uint8_t)60);
	// init adapter
	tcpip_adapter_init();
	// stop dhcp server
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
	// initialize event handling
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL)); // not sure if this is necessary
	// set default wifi configuration
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	// configure tcpip adapter
	tcpip_adapter_ip_info_t IpInfo;
	IpInfo.ip.addr = ipaddr_addr(sIPAdress.c_str());
	IpInfo.gw.addr = ipaddr_addr(sGWAdress.c_str());
	IpInfo.netmask.addr = ipaddr_addr(sNMAdress.c_str());
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &IpInfo));
	// start dhcp server
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	// configure default ap-params
	wifi_config_t ap_config = {};
	strcpy((char *)ap_config.ap.ssid, sSSID.c_str());
	ap_config.ap.ssid_len = strlen(sSSID.c_str());
	strcpy((char *)ap_config.ap.password, sPW.c_str());
	ap_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;
	ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	// set ap mode & config
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config)); // changed type here to WIFI_IF_AP (from ESP_IF_WIFI_STA)
	// log wifi configuration
	// u8_t *ip_display = (void *)&IpInfo.ip.addr;
	// ESP_LOGW(TAG, "Configured ESP with SSID: %s and PASSWORD: %s, IP: %d.%d.%d.%d", ap_config.ap.ssid, ap_config.ap.password, ip_display[0], ip_display[1], ip_display[2], ip_display[3]);
	ESP_LOGW(TAG, "Configured ESP with SSID: %s, PASSWORD: %s, IP: %s", ap_config.ap.ssid, ap_config.ap.password, sIPAdress.c_str());
	// start wifi in ap mode
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "started wifi in ap mode");
	// later maybe we will want to check for some errors?
	return ESP_OK;
}

esp_err_t MaagWifi::init_sta()
{
	ESP_LOGI(TAG, "initializing in sta mode");
	// wifi power
	esp_wifi_set_max_tx_power((uint8_t)60);
	// init adapter
	tcpip_adapter_init();
	// stop dhcp client
	ESP_ERROR_CHECK(tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA));
	// initialize event handling
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	// set default wifi configuration
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	// configure tcpip adapter out of remanent memory
	tcpip_adapter_ip_info_t IpInfo;
	IpInfo.ip.addr = ipaddr_addr(sIPAdress.c_str());
	IpInfo.gw.addr = ipaddr_addr(sGWAdress.c_str());
	IpInfo.netmask.addr = ipaddr_addr(sNMAdress.c_str());
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &IpInfo));

	//
	wifi_config_t sta_config = {};
	strcpy((char *)sta_config.sta.ssid, sSSID.c_str());
	strcpy((char *)sta_config.sta.password, sPW.c_str());
	sta_config.sta.bssid_set = false;

	// do not think next is needed...
	if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
	{
		sta_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	}
	// set sta mode & config
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
	//
	ESP_LOGW(TAG, "Configured ESP with SSID: %s, PASSWORD: %s, IP: %s", sta_config.sta.ssid, sta_config.sta.password, sIPAdress.c_str());
	// start wifi (maybe not needed, because done in main of wifi)
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(TAG, "started wifi in sta mode. Trying once to connect...");
	// later maybe we will want to check for some errors?
	return ESP_OK;
}

void MaagWifi::wifi_try_connect_sta()
{

	for (int i = 0; i < 20; i++)
	{
		ESP_LOGE(TAG, "try to disconnect wifi connection");
		ESP_ERROR_CHECK(esp_wifi_disconnect());
		ESP_LOGE(TAG, "disconnecting...");
		vTaskDelay((2000 / portTICK_PERIOD_MS));
		ESP_LOGE(TAG, "trying to connect to router");
		ESP_ERROR_CHECK(esp_wifi_connect());
		ESP_LOGE(TAG, "connecting... ");
		vTaskDelay((20000 / portTICK_PERIOD_MS));
		// vTaskDelay((5000));

		if (bConnectionStatus)
		{
			ESP_LOGE(TAG, "event handler signaling successful connection");
			break;
		}
		else if (i == 19)
		{
			ESP_LOGE(TAG, "timout connection to router! 20 unsuccessful connection attempts");
		}
		else
		{
			ESP_LOGE(TAG, "retry");
			;
		}
	}
}

bool MaagWifi::getConnectionStatus()
{
	return bConnectionStatus;
}

void MaagWifi::setIP(std::string IP_)
{
	sIPAdress = IP_;
}

void MaagWifi::setGW(std::string sGWAdress_)
{
	sGWAdress = sGWAdress_;
}

void MaagWifi::setNM(std::string sNMAdress_)
{
	sNMAdress = sNMAdress_;
}

void MaagWifi::setSSID(std::string sSSID_)
{
	sSSID = sSSID_;
}

void MaagWifi::setPW(std::string sPW_)
{
	sPW = sPW_;
}

// =====================================================================
// possible task that can be created in main
void maag_wifi_task(void *arg)
{

	while (1)
	{

		ESP_LOGW(TAG, "wifi task is alive but doing shit all");

		vTaskDelay((1000 / portTICK_PERIOD_MS));
	}
}
