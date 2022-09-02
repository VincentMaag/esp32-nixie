/*
    ...

*/
#ifndef __MAAG_SPI_DEVICE_H__
#define __MAAG_SPI_DEVICE_H__

//#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "maag_gpio.h"

class MaagSpiDevice
{
private:
    // spi device handle
    spi_device_handle_t m_device_handle;
    // gpio object for cs
    GpioOutput m_gpio_cs;

public:
    MaagSpiDevice();
    //MaagSpiDevice(spi_host_device_t host_, int clock_speed_hz_, gpio_num_t cs_gpio_nr_);
    
    // Create a SPI device and connect it to a spi-host. Initialize chip-select
    esp_err_t initDevice(spi_host_device_t host_, int clock_speed_hz_, gpio_num_t cs_gpio_nr_);
    // write to spi device
    esp_err_t write_bytes(uint8_t *out_data_, uint8_t out_data_size_);
    // select chip
    esp_err_t select()
    {
        return m_gpio_cs.setOutput(0);
    }
    // release chip
    esp_err_t release()
    {
        return m_gpio_cs.setOutput(1);
    }
};

#endif /* __MAAG_SPI_DEVICE_H__ */