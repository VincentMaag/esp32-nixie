/*
	...


*/

#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "driver/i2c.h"
#include "esp_log.h"

#include "maag_i2c_device.h"

// static TAG
static const char *TAG = "maag_i2c_device";



// =============================================================================================================
// CLASS MaagI2CDevice
// =============================================================================================================
MaagI2CDevice::MaagI2CDevice(/* args */)
{
	ESP_LOGW(TAG, "MaagI2C device instance created");
}

MaagI2CDevice::~MaagI2CDevice()
{
}

void MaagI2CDevice::setPort(i2c_port_t port_){
	m_port = port_;
}

void MaagI2CDevice::setDeviceAddress(uint8_t dev_addr_){
	m_dev_addr = dev_addr_;
}



esp_err_t MaagI2CDevice::read(uint8_t *out_data, size_t out_size, uint8_t *in_data, size_t in_size){

	//if (!dev || !in_data || !in_size) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (out_data && out_size) // ?
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (m_dev_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write(cmd, out_data, out_size, true);
    }

	i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (m_dev_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t res = i2c_master_cmd_begin(m_port, cmd, 1000 / portTICK_PERIOD_MS);
    if (res != ESP_OK)
        ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d", m_dev_addr, m_port, res);
    i2c_cmd_link_delete(cmd);

    return res;
}

uint8_t bcd2dec(uint8_t val)
{
    return (val >> 4) * 10 + (val & 0x0f);
}

uint8_t dec2bcd(uint8_t val)
{
    return ((val / 10) << 4) + (val % 10);
}


esp_err_t MaagI2CDevice::convertDisplayTime(uint8_t data[7])
{
	
	tm time_ = {0};

    /* convert to unix time structure */
    time_.tm_sec = bcd2dec(data[0]);
    time_.tm_min = bcd2dec(data[1]);
    if (data[2] & DS3231_12HOUR_FLAG)
    {
        /* 12H */
        time_.tm_hour = bcd2dec(data[2] & DS3231_12HOUR_MASK) - 1;
        /* AM/PM? */
        if (data[2] & DS3231_PM_FLAG) time_.tm_hour += 12;
    }
    else time_.tm_hour = bcd2dec(data[2]); /* 24H */
    time_.tm_wday = bcd2dec(data[3]) - 1;
    time_.tm_mday = bcd2dec(data[4]);
    time_.tm_mon  = bcd2dec(data[5] & DS3231_MONTH_MASK) - 1;
    time_.tm_year = bcd2dec(data[6]) + 0;
    time_.tm_isdst = 0;


	// time_t now = 0;
	// struct tm timeinfo = {0};
	// time(&now);

	// localtime_r(&now, &timeinfo);
	char strftime_buf[64];
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &time_);
	ESP_LOGW(TAG, "The current date/time is: %s", strftime_buf);


    // apply a time zone (if you are not using localtime on the rtc or you want to check/apply DST)
    //applyTZ(time);

    return ESP_OK;
}


// inline esp_err_t i2c_dev_read_reg(const i2c_dev_t *dev, uint8_t reg,
//         void *in_data, size_t in_size)
// {
//     return i2c_dev_read(dev, &reg, 1, in_data, in_size);
// }

// esp_err_t i2c_dev_read(const i2c_dev_t *dev, const void *out_data, size_t out_size, void *in_data, size_t in_size)
// {
//     if (!dev || !in_data || !in_size) return ESP_ERR_INVALID_ARG;

//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     if (out_data && out_size)
//     {
//         i2c_master_start(cmd);
//         i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, true);
//         i2c_master_write(cmd, (void *)out_data, out_size, true);
//     }
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (dev->addr << 1) | 1, true);
//     i2c_master_read(cmd, in_data, in_size, I2C_MASTER_LAST_NACK);
//     i2c_master_stop(cmd);

//     esp_err_t res = i2c_master_cmd_begin(dev->port, cmd, I2CDEV_TIMEOUT / portTICK_PERIOD_MS);
//     if (res != ESP_OK)
//         ESP_LOGE(TAG, "Could not read from device [0x%02x at %d]: %d", dev->addr, dev->port, res);
//     i2c_cmd_link_delete(cmd);

//     return res;
// }


// 