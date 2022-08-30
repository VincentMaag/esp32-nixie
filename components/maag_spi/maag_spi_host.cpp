/*
	...


*/

#include <string.h>
#include "esp_system.h"
#include "esp_log.h"

#include "maag_spi_host.h"


// static TAG
static const char *TAG = "maag_spi_host";

// =============================================================================================================
// CLASS MaagSpiHost
// =============================================================================================================
MaagSpiHost::MaagSpiHost()
{
		ESP_LOGW(TAG, "MaagSpiHost instance created");
}

esp_err_t MaagSpiHost::initHost(spi_host_device_t host_, gpio_num_t miso_gpio_nr_, gpio_num_t mosi_gpio_nr_, gpio_num_t sclk_gpio_nr_)
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
	//buscfg.max_transfer_sz = 64;		// lets not define a max size so it defaults to some size!
	return spi_bus_initialize(host_, &buscfg, SPI_DMA_DISABLED);
}

spi_host_device_t MaagSpiHost::getHostDevice(){
	return m_host_device;
}
