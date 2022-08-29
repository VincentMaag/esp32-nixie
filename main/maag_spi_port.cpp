/*
	...


*/

#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
// #include "driver/spi_master.h"

#include "maag_spi_port.h"


// static TAG
static const char *TAG = "maag_spi_port";

// =============================================================================================================
// CLASS MaagSpiPort
// =============================================================================================================
MaagSpiPort::MaagSpiPort(/* args */)
{
		ESP_LOGW(TAG, "MaagSpiPort instance created");
}

esp_err_t MaagSpiPort::initHost(spi_host_device_t host_, gpio_num_t miso_gpio_nr_, gpio_num_t mosi_gpio_nr_, gpio_num_t sclk_gpio_nr_)
{
	// copy configured host device
	m_host_device = host_;
	// configure and initialize host device
	spi_bus_config_t buscfg = {};
	buscfg.miso_io_num = miso_gpio_nr_;
	buscfg.mosi_io_num = mosi_gpio_nr_;
	buscfg.sclk_io_num = sclk_gpio_nr_;
	buscfg.quadwp_io_num = -1;
	buscfg.quadhd_io_num = -1;
	buscfg.max_transfer_sz = 64;
	return spi_bus_initialize(host_, &buscfg, SPI_DMA_DISABLED);
}

spi_host_device_t MaagSpiPort::getHostDevice(){
	return m_host_device;
}




// =============================================================================================================
// CLASS MaagSpiDevice
// =============================================================================================================
// static TAG
static const char *TAG2 = "maag_spi_device";

MaagSpiDevice::MaagSpiDevice()
{
	ESP_LOGW(TAG2, "MaagSpiDevice instance created");
}

esp_err_t MaagSpiDevice::initDevice(spi_host_device_t host_)
{

	spi_device_interface_config_t devcfg = {};

	devcfg.clock_speed_hz = 10000; // 10 kHz, will be roundet to closest possible (80Mhz/x = 10000)
	devcfg.mode = 0;				 // SPI mode 0
	devcfg.spics_io_num = -1;
	devcfg.queue_size = 1;
	devcfg.flags = SPI_DEVICE_HALFDUPLEX;
	devcfg.pre_cb = NULL;
	devcfg.post_cb = NULL;

	return spi_bus_add_device(host_, &devcfg, &m_device_handle);
}


esp_err_t MaagSpiDevice::write(uint8_t reg, uint8_t value)
{
	uint8_t tx_data[2] = {reg, value};

	spi_transaction_t t = {};
	t.tx_buffer = tx_data;
	t.length = 2 * 8;

	return spi_device_polling_transmit(m_device_handle, &t);
}