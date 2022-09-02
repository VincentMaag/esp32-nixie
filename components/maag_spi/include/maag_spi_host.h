/*
    ...

*/
#ifndef __MAAG_SPI_HOST_H__
#define __MAAG_SPI_HOST_H__

#include "driver/gpio.h"
#include "driver/spi_master.h"

// 
class MaagSpiHost
{
private:
    // SPI host number
    spi_host_device_t m_host_device;
public:
    MaagSpiHost();
    // init host device without a chip select
    esp_err_t initHost(spi_host_device_t host_, gpio_num_t miso_gpio_nr_, gpio_num_t mosi_gpio_nr_, gpio_num_t sclk_gpio_nr_);
    // get initialized host device
    spi_host_device_t getHostDevice();
};




#endif /* __MAAG_SPI_HOST_H__ */