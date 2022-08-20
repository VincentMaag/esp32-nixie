/*
    ...

*/
#ifndef __MAAG_I2C_PORT_H__
#define __MAAG_I2C_PORT_H__

#include "driver/i2c.h"

#define I2C_FREQ_HZ 400000

// class to create a I2C port
class MaagI2CPort
{
private:
    /* data */
    i2c_port_t m_port;
public:
    MaagI2CPort(/* args */);
    ~MaagI2CPort();
    // initialize i2c port
    esp_err_t initPort(i2c_port_t port_, gpio_num_t sda_, gpio_num_t scl_, i2c_mode_t mode_);
    // get i2c port number of current port
    i2c_port_t getPort();
};




#endif /* __MAAG_I2C_PORT_H__ */