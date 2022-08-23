/*
	...


*/
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"

#include "nixie_time.h"

// static TAG
static const char *TAG = "nixie_time";

NixieTime* NixieTime::m_staticThis;

// =============================================================================================================
// CLASS NixieTime
// =============================================================================================================
NixieTime::NixieTime(MaagSNTP &sntp_, DS3231 &ds3231_) : m_sntp(sntp_), m_ds3231(ds3231_)
{
	ESP_LOGW(TAG, "NixieTime Instance created");

	// lol, set our non-static this pointer to a static one, so that we can use it in a static callback
	// function that is handled in another class-instance ;)
	NixieTime::pointToThisInstance();
	// m_espTime = {};
	// m_ds3231Time = {};
}

struct tm NixieTime::getEspTime()
{
	struct timeval now = {};
	gettimeofday(&now, NULL);
	localtime_r(&(now.tv_sec), &m_espTime);
	return m_espTime;
}

struct tm NixieTime::getDs3231Time()
{
	m_ds3231Time = m_ds3231.getTime();
	return m_ds3231Time;
}

char *NixieTime::getEspTimeAsString()
{
	NixieTime::getEspTime();
	strftime(m_strftime_buf_esp, sizeof(m_strftime_buf_esp), "%c", &m_espTime);
	return m_strftime_buf_esp;
}

char *NixieTime::getDs3231TimeAsString()
{
	NixieTime::getDs3231Time();
	strftime(m_strftime_buf_ds3231, sizeof(m_strftime_buf_ds3231), "%c", &m_ds3231Time);
	return m_strftime_buf_ds3231;
}

esp_err_t NixieTime::synchTime(nixie_time_master_t master_){

	// if esp is chosen as Master, then we set ds3231 time equal to esp system time
	// Vise versa if ds3231 has been chosen as Master
	if(master_ == NIXIE_TIME_ESP_AS_MASTER){
		// get esp time, set ds3231 time
		m_ds3231.setTime(NixieTime::getEspTime());
		ESP_LOGI(TAG, "DS3231 time has been synchronized to ESP-System time");
	}else if(master_ == NIXIE_TIME_DS3231_AS_MASTER){
		// get ds3231 time
		NixieTime::getDs3231Time();
		// prepare temp variable
		struct timeval now;
		// refactor tm into time_t, set seconds of timeval
		now.tv_sec = mktime(&m_ds3231Time);
		// set esp32 system time
		settimeofday(&now, NULL);
		ESP_LOGI(TAG, "ESP-System time has been synchronized to DS3231 time");
	}

	return ESP_OK;
}

time_t NixieTime::getTimeDifference()
{
	// get current Times
	NixieTime::getEspTime();
	NixieTime::getDs3231Time();
	// calculate difference in s
	time_t esp_time_t = mktime(&m_espTime);
	time_t ds3231_time_t = mktime(&m_ds3231Time);
	return abs(esp_time_t - ds3231_time_t);
}

esp_err_t NixieTime::synchTimeIfDiffLargerThan(time_t allowedTimeDiff_, nixie_time_master_t master_)
{
	time_t timeDiff = NixieTime::getTimeDifference();
	if(timeDiff > abs(allowedTimeDiff_)){
		// if times are not syncronized, we set esp equal to ds3231 because we trust
		// ds3231 more. Of course this only holds if ds3231 has not been reset
		ESP_LOGW(TAG, "ESP-System and DS3231 time have a difference of %ld",timeDiff);
		return NixieTime::synchTime(master_);
	}else{
		// if difference is not greater than user argument, do nothing
		ESP_LOGW(TAG, "ESP-System and DS3231 time have a difference which is in the boundaries of %ld",allowedTimeDiff_);
		return ESP_OK;
	}
	return ESP_FAIL;
}
    
 esp_err_t NixieTime::createSynchTask(time_t synchTaskAllowedTimeDiff_, nixie_time_master_t synchTaskMaster_, TickType_t synchTaskticksToDelay_, BaseType_t xCoreID_){
	
	// copy member variables for our synchTask
	m_synchTaskAllowedTimeDiff = synchTaskAllowedTimeDiff_;
	m_synchTaskMaster = synchTaskMaster_;
	m_synchTaskticksToDelay = synchTaskticksToDelay_;
	// only create this task once!
	xTaskCreatePinnedToCore(NixieTime::nixie_time_task, "nixie_time_task", 4000, this, 5, NULL, xCoreID_);
	return ESP_OK;

 }

 void NixieTime::nixie_time_task(void *pArgs)
 {
	 const static char *TAG2 = "nixie_time_task";
	 ESP_LOGW(TAG2, "nixie time synch handling task created");
	 // cast my argument to what I know it is, because we created the task and past "this" as the argument pointer
	 NixieTime *pNixieTime = (NixieTime *)pArgs;

	 while (1)
	 {
		// check to synchronize
		// --> we could also use m_staticThis actually... put hey, there are always multiple ways to solve problems!
		 pNixieTime->synchTimeIfDiffLargerThan(pNixieTime->getSynchTaskAllowedTimeDiff(), pNixieTime->getSynchTaskMaster());
		 // log
		 pNixieTime->logTimes();
		 // wait for next round
		 vTaskDelay((pNixieTime->getSynchTaskticksToDelay() / portTICK_PERIOD_MS));
	 }
 }


void NixieTime::nixieTimeSNTPSyncNotificationCb(struct timeval *tv)
{
	// sntp has thrown callback for a time synch event
	ESP_LOGW(TAG, "Notification of a time synchronization event. The current date/time is: %s", m_staticThis->getEspTimeAsString());
	// ok, now lets synchronize ds3231 with esp time because we now have the correct time!
	m_staticThis->synchTime(NIXIE_TIME_ESP_AS_MASTER);
	// log
	m_staticThis->logTimes();
}


void NixieTime::logTimes()
{
	ESP_LOGW(TAG, "ESP-SYSTEM TIME: %s === DS3231-DEVICE TIME: %s", NixieTime::getEspTimeAsString(), NixieTime::getDs3231TimeAsString());
}
