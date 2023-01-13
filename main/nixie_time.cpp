/*
	...
	Make my own mktime for gmt representation?

	switch from localtime_r to localtime for readability

*/
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"

#include "nixie_time.h"

// static TAG
static const char *TAG = "nixie_time";
// we need this one for our static callback function that is passed to sntp instance
NixieTime *NixieTime::m_staticThis;

// =============================================================================================================
// CLASS NixieTime
// =============================================================================================================
NixieTime::NixieTime(MaagSNTP &sntp_, DS3231 &ds3231_) : m_sntp(sntp_), m_ds3231(ds3231_)
{
	ESP_LOGW(TAG, "NixieTime Instance created");
	// Set our non-static "this" pointer to a static one, so that we can use it in a static callback function. This
	// is needed because we want to manipulate class-instance stuff in the callback functions
	NixieTime::pointToThisInstance();
	// set timezone
	NixieTime::setTimeZone();
	// initialize sntp with our custom static callback and start sntp service
	m_sntp.setSyncNotificationCb(NixieTime::nixieTimeSNTPSyncNotificationCb);
	m_sntp.initStart();
}

void NixieTime::setTimeZone()
{
	setenv("TZ", "MEZ-1MESZ", 1);
	// setenv("TZ", "GMT0BST", 1);
	tzset();
}

struct tm NixieTime::getEspTime(esp_time_zone_t zone_)
{
	struct tm temp = {};
	// get current esp system time
	time_t now_as_time_t = time(0);

	if (zone_ == ESP_TIME_LOCAL)
	{
		localtime_r(&now_as_time_t, &temp);
		return temp;
	}
	else if(zone_ == ESP_TIME_GMT)
	{
		gmtime_r(&now_as_time_t, &temp);
		return temp;
	}else{
		struct tm empty = {};
		return empty;
	}
}

struct tm NixieTime::getDs3231Time()
{
	return m_ds3231.getTime();
}

char *NixieTime::getEspTimeAsString(esp_time_zone_t zone_)
{
	struct tm espTime = NixieTime::getEspTime(zone_);
	strftime(m_strftime_buf_esp[zone_], sizeof(m_strftime_buf_esp), "%c", &espTime);
	return m_strftime_buf_esp[zone_];
}

char *NixieTime::getDs3231TimeAsString()
{
	struct tm ds3231Time = NixieTime::getDs3231Time();
	strftime(m_strftime_buf_ds3231, sizeof(m_strftime_buf_ds3231), "%c", &ds3231Time);
	return m_strftime_buf_ds3231;
}

esp_err_t NixieTime::synchTime(nixie_time_master_t master_)
{

	// if esp is chosen as Master, then we set ds3231 time equal to esp system time
	// Vise versa if ds3231 has been chosen as Master
	if (master_ == NIXIE_TIME_ESP_AS_MASTER)
	{
		// get esp time, set ds3231 time
		m_ds3231.setTime(NixieTime::getEspTime(ESP_TIME_GMT));
		ESP_LOGI(TAG, "DS3231 time has been synchronized to ESP-System time");
	}
	else if (master_ == NIXIE_TIME_DS3231_AS_MASTER)
	{
		// get ds3231 time
		struct tm nixie_tm = NixieTime::getDs3231Time();
		// prepare temp variable
		struct timeval now;
		// refactor tm into time_t, set seconds of timeval
		//now.tv_sec = mktime(&m_ds3231Time);

		time_t temp = mktime(&nixie_tm);
		time_t ds3231_time_t = temp + (mktime(localtime(&temp)) - mktime(gmtime(&temp))); // this is the workaraound since there is no function like mktime for gmt time...
		now.tv_sec = ds3231_time_t;

		// set esp32 system time
		settimeofday(&now, NULL);
		ESP_LOGI(TAG, "ESP-System time has been synchronized to DS3231 time");
	}

	return ESP_OK;
}

time_t NixieTime::getTimeDifference()
{
	// get current Times
	struct tm esp_tm = NixieTime::getEspTime(ESP_TIME_GMT);
	struct tm espLocal_tm = NixieTime::getEspTime(ESP_TIME_LOCAL);
	struct tm ds3231_tm = NixieTime::getDs3231Time();

	// ESP_LOGI(TAG, "ESP GMT: sec: %i, min: %i, hour: %i, mday: %i, mon: %i, year: %i, wday: %i, yday: %i, isdst: %i", esp_tm.tm_sec, esp_tm.tm_min, esp_tm.tm_hour, esp_tm.tm_mday, esp_tm.tm_mon, esp_tm.tm_year, esp_tm.tm_wday, esp_tm.tm_yday, esp_tm.tm_isdst);
	// ESP_LOGI(TAG, "ESP LOCAL: sec: %i, min: %i, hour: %i, mday: %i, mon: %i, year: %i, wday: %i, yday: %i, isdst: %i", espLocal_tm.tm_sec, espLocal_tm.tm_min, espLocal_tm.tm_hour, espLocal_tm.tm_mday, espLocal_tm.tm_mon, espLocal_tm.tm_year, espLocal_tm.tm_wday, espLocal_tm.tm_yday, espLocal_tm.tm_isdst);
	// ESP_LOGI(TAG, "DS3231: sec: %i, min: %i, hour: %i, mday: %i, mon: %i, year: %i, wday: %i, yday: %i, isdst: %i", ds3231_tm.tm_sec, ds3231_tm.tm_min, ds3231_tm.tm_hour, ds3231_tm.tm_mday, ds3231_tm.tm_mon, ds3231_tm.tm_year, ds3231_tm.tm_wday, ds3231_tm.tm_yday, ds3231_tm.tm_isdst);

	// time_t esp_time_t = mktime(&esp_tm);
	// time_t ds3231_time_t = mktime(&ds3231_tm);

	time_t temp = mktime(&esp_tm);
	time_t esp_time_t = temp + (mktime(localtime(&temp)) - mktime(gmtime(&temp))); // this is the workaraound since there is no function like mktime for gmt time...
	
	temp = mktime(&ds3231_tm);
	time_t ds3231_time_t = temp + (mktime(localtime(&temp)) - mktime(gmtime(&temp))); // this is the workaraound since there is no function like mktime for gmt time...
	
	//ESP_LOGI(TAG, "esp in s: %lu, ds3231 in s: %lu",esp_time_t, ds3231_time_t);

	return abs(esp_time_t - ds3231_time_t);
}

esp_err_t NixieTime::synchTimeIfDiffLargerThan(time_t allowedTimeDiff_, nixie_time_master_t master_)
{
	time_t timeDiff = NixieTime::getTimeDifference();
	if (timeDiff > abs(allowedTimeDiff_))
	{
		ESP_LOGW(TAG, "ESP-System and DS3231 time have a difference of %ld. Attempting to syncronize time", timeDiff);
		return NixieTime::synchTime(master_);
	}
	else
	{
		ESP_LOGI(TAG, "ESP-System and DS3231 time have a difference which is in the boundaries of %ld. No syncronization needed.", allowedTimeDiff_);
		return ESP_OK;
	}
	return ESP_FAIL;
}

esp_err_t NixieTime::createSynchTask(time_t synchTaskAllowedTimeDiff_, nixie_time_master_t synchTaskMaster_, TickType_t synchTaskticksToDelay_, BaseType_t xCoreID_)
{
	// copy parameters to our member variables. These will be visible in the nixie time task because we pass the "this" pointer when creating a FreeRtos Task!
	m_synchTaskAllowedTimeDiff = synchTaskAllowedTimeDiff_;
	m_synchTaskMaster = synchTaskMaster_;
	m_synchTaskticksToDelay = synchTaskticksToDelay_;
	// only create this task once!
	xTaskCreatePinnedToCore(NixieTime::nixie_time_task, "nixie_time_task", 4000, this, 0, NULL, xCoreID_);
	ESP_LOGI(TAG, "nixie_time_task created");
	return ESP_OK;
}

void NixieTime::nixie_time_task(void *pArgs)
{
	const static char *TAG2 = "nixie_time_task";
	// cast my argument to what I know it is, because we created the task and past "this" as the argument pointer
	NixieTime *pNixieTime = (NixieTime *)pArgs;
	// Timekeeping
	TickType_t previousWakeTime = xTaskGetTickCount();
	while (1)
	{
		// check to synchronize
		pNixieTime->synchTimeIfDiffLargerThan(pNixieTime->getSynchTaskAllowedTimeDiff(), pNixieTime->getSynchTaskMaster());
		// log
		pNixieTime->logTimes();
		// wait for next round
		xTaskDelayUntil(&previousWakeTime, (pNixieTime->getSynchTaskticksToDelay() / portTICK_PERIOD_MS));
	}
}

void NixieTime::nixieTimeSNTPSyncNotificationCb(struct timeval *tv)
{
	// sntp has thrown callback for a time synch event
	ESP_LOGW(TAG, "Notification of a time synchronization event. The current date/times are: LOCAL %s, GMT %s", m_staticThis->getEspTimeAsString(ESP_TIME_LOCAL), m_staticThis->getEspTimeAsString(ESP_TIME_GMT));
	// ok, now lets synchronize ds3231 with esp time because we now have the correct time!
	
	
	m_staticThis->synchTime(NIXIE_TIME_ESP_AS_MASTER);
	
	
	// log
	m_staticThis->logTimes();
}

void NixieTime::logTimes()
{
	ESP_LOGI(TAG, "ESP-LOCAL: %s === ESP-GMT: %s === DS3231: %s", NixieTime::getEspTimeAsString(ESP_TIME_LOCAL), NixieTime::getEspTimeAsString(ESP_TIME_GMT), NixieTime::getDs3231TimeAsString());	

}



