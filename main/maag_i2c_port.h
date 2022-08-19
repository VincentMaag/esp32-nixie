/*
    ...

*/
#ifndef __MAAG_I2C_PORT_H__
#define __MAAG_I2C_PORT_H__


#include "driver/i2c.h"

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
    esp_err_t initPort(i2c_port_t port_, gpio_num_t sda_, gpio_num_t scl_);
    // get i2c port
    i2c_port_t getPort();
};




#endif /* __MAAG_I2C_PORT_H__ */