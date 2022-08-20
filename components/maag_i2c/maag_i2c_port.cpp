/*
	...


*/

#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "maag_i2c_port.h"

// static TAG
static const char *TAG = "maag_i2c_port";


// =============================================================================================================
// CLASS MaagI2CPort
// =============================================================================================================
MaagI2CPort::MaagI2CPort(/* args */)
{
	ESP_LOGW(TAG, "MaagI2C port instance created");
}

MaagI2CPort::~MaagI2CPort()
{
}

esp_err_t MaagI2CPort::initPort(i2c_port_t port_, gpio_num_t sda_, gpio_num_t scl_, i2c_mode_t mode_)
{
	ESP_LOGI(TAG, "initializing I2C Port");

	m_port = port_;

	i2c_config_t i2c_config = {};
	i2c_config.mode = mode_;
	i2c_config.sda_io_num = sda_;
	i2c_config.scl_io_num = scl_;
	i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
	i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
	i2c_config.master.clk_speed = I2C_FREQ_HZ;
	i2c_param_config(port_, &i2c_config);
	return i2c_driver_install(port_, I2C_MODE_MASTER, 0, 0, 0);
}

i2c_port_t MaagI2CPort::getPort(){
	return m_port;
}





