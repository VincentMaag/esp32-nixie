/*
	...


*/
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"

#include "nixie_time.h"

// static TAG
static const char *TAG = "nixie_time";

// =============================================================================================================
// CLASS NixieTime
// =============================================================================================================
NixieTime::NixieTime(MaagSNTP &sntp_, DS3231 &ds3231_) : m_sntp(sntp_), m_ds3231(ds3231_)
{
	ESP_LOGW(TAG, "NixieTime Instance created");
	m_espTime = {0};
	m_ds3231Time = {0};
}

NixieTime::~NixieTime()
{
}

struct tm NixieTime::getEspTime()
{
	time_t now = 0;
	time(&now);
	localtime_r(&now, &m_espTime);
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
		ESP_LOGW(TAG, "DS3231 time has been synchronized to ESP-System time");
	}else if(master_ == NIXIE_TIME_DS3231_AS_MASTER){
		// get ds3231 time
		NixieTime::getDs3231Time();
		// prepare temp variable
		timeval now;
		// refactor tm into time_t, set seconds of timeval
		now.tv_sec = mktime(&m_ds3231Time);
		

		//mktime(&(NixieTime::getDs3231Time()))

		// set esp32 system time
		settimeofday(&now, NULL);
		ESP_LOGW(TAG, "ESP-System time has been synchronized to DS3231 time");
	}

	return ESP_OK;
}


void NixieTime::doSomething()
{


	// get esp and ds3231 time
	ESP_LOGI(TAG, "The current date/times are: === ESP-SYSTEM: %s === DS3231-DEVICE: %s", NixieTime::getEspTimeAsString(), NixieTime::getDs3231TimeAsString());
	// wait a second, then synchronize esp time to ds3231 time
	vTaskDelay((100 / portTICK_PERIOD_MS));
	NixieTime::synchTime(NIXIE_TIME_DS3231_AS_MASTER);

}
