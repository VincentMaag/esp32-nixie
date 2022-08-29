/*
    ...

*/
#ifndef __MAAG_SPI_PORT_H__
#define __MAAG_SPI_PORT_H__

#include "driver/gpio.h"
#include "driver/spi_master.h"

// 
class MaagSpiPort
{
private:
    // SPI host number
    spi_host_device_t m_host_device;
public:
    MaagSpiPort();
    // init host device
    esp_err_t initHost(spi_host_device_t host_, gpio_num_t miso_gpio_nr_, gpio_num_t mosi_gpio_nr_, gpio_num_t sclk_gpio_nr_);
    // get initialized host device
    spi_host_device_t getHostDevice();
};


// ====================================================================================

class MaagSpiDevice
{
private:

    spi_device_handle_t m_device_handle;

public:
    MaagSpiDevice();


    esp_err_t initDevice(spi_host_device_t host_);


    esp_err_t write(uint8_t reg, uint8_t value);



};











#endif /* __MAAG_SPI_PORT_H__ */