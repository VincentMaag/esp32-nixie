/*
	...


*/

#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
// #include "driver/spi_master.h"

#include "maag_spi_device.h"

// static TAG
static const char *TAG2 = "maag_spi_device";

// =============================================================================================================
// CLASS MaagSpiDevice
// =============================================================================================================
MaagSpiDevice::MaagSpiDevice()
{
	ESP_LOGW(TAG2, "MaagSpiDevice instance created");
}
// MaagSpiDevice::MaagSpiDevice(spi_host_device_t host_, int clock_speed_hz_, gpio_num_t cs_gpio_nr_) : m_gpio_cs(cs_gpio_nr_, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE)
// {
// 	ESP_LOGW(TAG2, "MaagSpiDevice instance created");
// 	MaagSpiDevice::initDevice(host_, clock_speed_hz_);
// }

// ToDo: handle lots of init parameters here
esp_err_t MaagSpiDevice::initDevice(spi_host_device_t host_, int clock_speed_hz_, gpio_num_t cs_gpio_nr_)
{
	// init cs gpio
	m_gpio_cs.initAsOutput(cs_gpio_nr_, GPIO_PULLDOWN_DISABLE, GPIO_PULLUP_ENABLE);
	// init spi device and connect to host
	spi_device_interface_config_t devcfg = {};
	devcfg.clock_speed_hz = clock_speed_hz_; // 10 kHz, will be roundet to closest possible (80Mhz/x = 10000)
	devcfg.mode = 0;						 // SPI mode 0
	devcfg.spics_io_num = -1;				 // no chip select, we handle this ourselves
	devcfg.queue_size = 1;
	// devcfg.flags = SPI_DEVICE_HALFDUPLEX;		// lets not change any default modes...
	devcfg.command_bits = 0;
	devcfg.address_bits = 0;
	devcfg.pre_cb = NULL;
	devcfg.post_cb = NULL;
	// some other specs that still have to be defined in future
	return spi_bus_add_device(host_, &devcfg, &m_device_handle);
}

esp_err_t MaagSpiDevice::write_bytes(uint8_t *out_data_, uint8_t out_data_size_)
{
	// create a transaction object and just pass arguments and size
	spi_transaction_t t = {};
	t.tx_buffer = out_data_;
	t.length = out_data_size_;
	// fire
	return spi_device_polling_transmit(m_device_handle, &t);
}