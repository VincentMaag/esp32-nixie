/*
    ...

*/
#ifndef __MAAG_I2C_DEVICE_H__
#define __MAAG_I2C_DEVICE_H__

#include <time.h>
#include "driver/i2c.h"


// #define DS3231_12HOUR_FLAG  0x40
// #define DS3231_12HOUR_MASK  0x1f
// #define DS3231_PM_FLAG      0x20
// #define DS3231_MONTH_MASK   0x1f

#define I2CDEV_TIMEOUT 1000

// class to create a I2C device

class MaagI2CDevice
{
private:
    // device port
    i2c_port_t m_port;
    // device address
    uint8_t m_dev_addr;

public:
    MaagI2CDevice();
    // select port that must already be initialized
    void setPort(i2c_port_t port_);
    // set device address
    void setDeviceAddress(uint8_t dev_addr_);
    // read from device
    esp_err_t read(uint8_t *out_data, size_t out_size, uint8_t *in_data, size_t in_size);
    // write to device
    esp_err_t write(uint8_t *data_reg_, size_t data_reg_size_, uint8_t *out_data_, size_t out_data_size_);

};




#endif /* __MAAG_I2C_DEVICE_H__ */