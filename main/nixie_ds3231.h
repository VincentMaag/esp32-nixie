/*
    ...

*/
#ifndef __NIXIE_DS3231_H__
#define __NIXIE_DS3231_H__

#include "maag_i2c_device.h"


// class for a DS3231 RTC Module
class DS3231 : public MaagI2CDevice
{
private:
    
    struct tm m_ds3231Time = {};
    uint8_t bcd2dec(uint8_t arg_);
    uint8_t dec2bcd(uint8_t arg_);

public:
    DS3231(i2c_port_t port_);
    // get time of ds3231
    struct tm getTime();
    // set time of ds3231 to a user specific time
    esp_err_t setTime(struct tm time_);
    // set time of ds3231 to esp32 system time
    esp_err_t setTimeToEspSystemTime();

};










#endif /* __NIXIE_DS3231_H__ */