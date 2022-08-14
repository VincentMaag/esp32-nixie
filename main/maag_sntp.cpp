/*
	...


*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "lwip/ip4_addr.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/api.h"



// from example
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
//#include "protocol_examples_common.h"
#include "esp_sntp.h"

// from old
// #include "time.h"
//#include "sys/time.h"


#include "maag_sntp.h"

// static TAG
static const char *TAG = "maag_sntp";

static int first = 0;

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void initialize_sntp_maag(void)
{


	// #ifdef LWIP_DHCP_GET_NTP_SRV
    // 	sntp_servermode_dhcp(1);
	// #endif

	if(first < 1){
		ESP_LOGW(TAG, "Initializing SNTP");
    	sntp_setoperatingmode(SNTP_OPMODE_POLL);
    	sntp_setservername(0, "pool.ntp.org");
		// sntp_setservername(0, "0.ch.pool.ntp.org");
    	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
		sntp_init();
		ESP_LOGW(TAG, "INIT DONE...");
	}else{
		ESP_LOGW(TAG, "INIT ALREADY DONE...");
	}

	first = 1;

	

	// wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
	uint32_t sec = 0;
	uint32_t us = 0;
	
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGW(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		ESP_LOGW(TAG, "Status is %i, this silly define is: %i", (int)sntp_get_sync_status(), SNTP_SERVER_DNS);
		sntp_get_system_time(&sec, &us);
		ESP_LOGW(TAG, "sntp get time: %i, %i", sec, us);
		
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);


	char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGW(TAG, "The current date/time is: %s", strftime_buf);


	

}






