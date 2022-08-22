/*
    ...

*/
#ifndef __NIXIE_TIME_H__
#define __NIXIE_TIME_H__

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include <string>

#include "maag_sntp.h"
#include "nixie_ds3231.h"

typedef enum {
    NIXIE_TIME_ESP_AS_MASTER = 0,
    NIXIE_TIME_DS3231_AS_MASTER
} nixie_time_master_t;

class NixieTime
{
private:
    // references to our sntp object, with which we can manipulate
    // our callbakc function(s) when we synch time
    MaagSNTP &m_sntp;
    // reference to our ds3231 objetc, with which we can get our RTC time
    DS3231 &m_ds3231;
    // current esp system time
    struct tm m_espTime;
    char m_strftime_buf_esp[64] = {0};
    // current ds3231 time
    struct tm m_ds3231Time;
    char m_strftime_buf_ds3231[64] = {0};

    /* data */
public:
    NixieTime(MaagSNTP &sntp_, DS3231 &ds3231_);
    ~NixieTime();

    // set which timesource is our master time
    esp_err_t setMasterTime();
    // get Master source time
    struct tm getMasterTime();



    // syncronize Times between esp and ds3231. <Parameter>-Source will set <other>-Source
    esp_err_t synchTime(nixie_time_master_t master_);


    // get current esp time
    struct tm getEspTime();
    // get current esp time as a char pointer
    char *getEspTimeAsString();
    // get current ds3231 time
    struct tm getDs3231Time();
    // get current ds3231 time as a char pointer
    char *getDs3231TimeAsString();

    //
    void doSomething();
};

#endif /* __NIXIE_TIME_H__ */