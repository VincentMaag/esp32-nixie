/*
	...


*/

#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include <time.h>
#include <sys/time.h>

#include "nixie_ds3231.h"

#define DS3231_ADDR 0x68 //!< I2C address

#define DS3231_STAT_OSCILLATOR 0x80
#define DS3231_STAT_32KHZ 0x08
#define DS3231_STAT_BUSY 0x04
#define DS3231_STAT_ALARM_2 0x02
#define DS3231_STAT_ALARM_1 0x01

#define DS3231_CTRL_OSCILLATOR 0x80
#define DS3231_CTRL_SQUAREWAVE_BB 0x40
#define DS3231_CTRL_TEMPCONV 0x20
#define DS3231_CTRL_ALARM_INTS 0x04
#define DS3231_CTRL_ALARM2_INT 0x02
#define DS3231_CTRL_ALARM1_INT 0x01

#define DS3231_ALARM_WDAY 0x40
#define DS3231_ALARM_NOTSET 0x80

#define DS3231_ADDR_TIME 0x00
#define DS3231_ADDR_ALARM1 0x07
#define DS3231_ADDR_ALARM2 0x0b
#define DS3231_ADDR_CONTROL 0x0e
#define DS3231_ADDR_STATUS 0x0f
#define DS3231_ADDR_AGING 0x10
#define DS3231_ADDR_TEMP 0x11

#define DS3231_12HOUR_FLAG 0x40
#define DS3231_12HOUR_MASK 0x1f
#define DS3231_PM_FLAG 0x20
#define DS3231_MONTH_MASK 0x1f

// static TAG
static const char *TAG = "nixie_ds3231";

// =============================================================================================================
// CLASS DS3231
// =============================================================================================================
DS3231::DS3231(i2c_port_t port_)
{
	MaagI2CDevice::setPort(port_);
	MaagI2CDevice::setDeviceAddress(DS3231_ADDR);
	ESP_LOGW(TAG, "DS3231 instance created");
}

uint8_t DS3231::bcd2dec(uint8_t arg_)
{
	return (arg_ >> 4) * 10 + (arg_ & 0x0f);
}

uint8_t DS3231::dec2bcd(uint8_t arg_)
{
	return ((arg_ / 10) << 4) + (arg_ % 10);
}

struct tm DS3231::getTime()
{
	uint8_t time_reg = DS3231_ADDR_TIME;
	uint8_t data[7] = {};
	// get time of i2c device
	// ESP_LOGI(TAG, "Reading time from ds3231");
	if(MaagI2CDevice::read(&time_reg, 1, &data[0], 7) != ESP_OK){
		return m_ds3231Time;
	};

	/* convert to unix time structure */
	m_ds3231Time.tm_sec = DS3231::bcd2dec(data[0]);
	m_ds3231Time.tm_min = DS3231::bcd2dec(data[1]);
	if (data[2] & DS3231_12HOUR_FLAG)
	{
		/* 12H */
		m_ds3231Time.tm_hour = DS3231::bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
		/* AM/PM? */
		if (data[2] & DS3231_PM_FLAG)
			m_ds3231Time.tm_hour += 12;
	}
	else
		m_ds3231Time.tm_hour = DS3231::bcd2dec(data[2]); /* 24H */
	m_ds3231Time.tm_wday = DS3231::bcd2dec(data[3]) - 1;
	m_ds3231Time.tm_mday = DS3231::bcd2dec(data[4]);
	m_ds3231Time.tm_mon = DS3231::bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
	m_ds3231Time.tm_year = DS3231::bcd2dec(data[6]) + 0;
	m_ds3231Time.tm_isdst = 1;

	return m_ds3231Time;
}

esp_err_t DS3231::setTime(struct tm time_)
{
	uint8_t time_reg = DS3231_ADDR_TIME;
	uint8_t data[7] = {0};
	/* time/date data */
	data[0] = DS3231::dec2bcd(time_.tm_sec);
	data[1] = DS3231::dec2bcd(time_.tm_min);
	data[2] = DS3231::dec2bcd(time_.tm_hour);
	/* The week data must be in the range 1 to 7, and to keep the start on the
	 * same day as for tm_wday have it start at 1 on Sunday. */
	data[3] = DS3231::dec2bcd(time_.tm_wday + 1);
	data[4] = DS3231::dec2bcd(time_.tm_mday);
	data[5] = DS3231::dec2bcd(time_.tm_mon + 1);
	data[6] = DS3231::dec2bcd(time_.tm_year - 0);

	//ESP_LOGI(TAG, "Writting time to ds3231");
	return MaagI2CDevice::write(&time_reg, 1, data, 7);
}

esp_err_t DS3231::setTimeToEspSystemTime()
{
	struct timeval now = {};
	gettimeofday(&now, NULL);
	struct tm espTime = {};
	// localtime_r(&(now.tv_sec), &espTime);
	gmtime_r(&(now.tv_sec), &espTime);
	// set ds3231 time
	return DS3231::setTime(espTime);
}
