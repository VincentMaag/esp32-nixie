/*
	...


*/

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"

#include "maag_sntp.h"

// static TAG
static const char *TAG = "maag_sntp";

// our function pointer
sntp_sync_time_cb_t MaagSNTP::m_callback_func;
// static int first = 0;
uint8_t MaagSNTP::ui8firstInit = 0;

// =============================================================================================================
// CLASS MaagSNTP
// =============================================================================================================
MaagSNTP::MaagSNTP(/* args */)
{
	ESP_LOGW(TAG, "MaagSNTP Instance created. Setting default interval and callback function");
	m_ui32interval_ms = 15000;
	m_callback_func = MaagSNTP::defaultSyncNotificationCb;
}

void MaagSNTP::setSyncNotificationCb(sntp_sync_time_cb_t callback_func_){
	ESP_LOGI(TAG, "Custom SNTP synch notification callback set");
	// let our function pointer look in the right direction
	m_callback_func = callback_func_;
}

void MaagSNTP::setSynchInterval(uint32_t ui32interval_ms_){
	m_ui32interval_ms = ui32interval_ms_;
}

void MaagSNTP::initStart(){

	// only init and start sntp if it hasn't already been started
	if (MaagSNTP::ui8firstInit < 1)
	{
		ESP_LOGI(TAG, "Initializing SNTP");
		sntp_setoperatingmode(SNTP_OPMODE_POLL);
		sntp_setservername(0, "pool.ntp.org");
		sntp_set_sync_interval(m_ui32interval_ms);
		sntp_set_time_sync_notification_cb(m_callback_func);
		sntp_init();
		ESP_LOGI(TAG, "SNTP initialized");
	}
	else
	{
		ESP_LOGE(TAG, "SNTP already initialized! Cannot do this twice...");
	}
	MaagSNTP::ui8firstInit = 1;
}

void MaagSNTP::defaultSyncNotificationCb(struct timeval *tv)
{
	// new tm structure for convenience
	struct tm timeinfo = {};
	// esp system time is passed in callback. Convert to tm
	localtime_r(&(tv->tv_sec), &timeinfo);
	// convert to a string
	char strftime_buf[64];
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	// and log
	ESP_LOGW(TAG, "Notification of a time synchronization event. The current date/time is: %s", strftime_buf);
}
