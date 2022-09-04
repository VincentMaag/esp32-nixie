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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "maag_sntp.h"
#include "nixie_ds3231.h"

typedef enum {
    ESP_TIME_GMT = 0,
    ESP_TIME_LOCAL
} esp_time_zone_t;

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
    // current esp system time (GMT)
    struct tm m_espTime = {};
    // current local esp system time (defined by timezone)
    struct tm m_espLocalTime = {};
    //
    char m_strftime_buf_esp[2][64] = {{}, {'\0'}};
    // current ds3231 time
    struct tm m_ds3231Time = {};
    char m_strftime_buf_ds3231[64] = {'\0'};
    // current local time offset in [s]
    int m_local_time_offset = 0;
    // allowed time difference in [s]
    time_t m_synchTaskAllowedTimeDiff = 0;
    // who is master 
    nixie_time_master_t m_synchTaskMaster = NIXIE_TIME_DS3231_AS_MASTER;
    // time between synch between master and slave in [ms]
    TickType_t m_synchTaskticksToDelay = 4000;
    // our esp32 synchronisation task that will do its job in the backgorund
    static void nixie_time_task(void * pArgs);
    // lets be cheeky here. Lets create a static pointer that we will point to an instance of this class
    static NixieTime* m_staticThis;

    // function at startp to create a static pointer to last created instance of this class
    void pointToThisInstance(){
        m_staticThis = this;
    }

    // our sntp callback function that will do the ds3231 synchronisation
    static void nixieTimeSNTPSyncNotificationCb(struct timeval *tv);
    // simple getters
    time_t getSynchTaskAllowedTimeDiff(){
        return m_synchTaskAllowedTimeDiff;
    }
    nixie_time_master_t getSynchTaskMaster(){
        return m_synchTaskMaster;
    }
    TickType_t getSynchTaskticksToDelay(){
        return m_synchTaskticksToDelay;
    }

public:
    NixieTime(MaagSNTP &sntp_, DS3231 &ds3231_);
    // syncronize Times between esp and ds3231. <Parameter>-Source will set <other>-Source
    esp_err_t synchTime(nixie_time_master_t master_);
    // get current esp time
    struct tm getEspTime(esp_time_zone_t zone_);
    // set Timezone
    void setTimeZone();
    // get current esp time as a char pointer
    char *getEspTimeAsString(esp_time_zone_t zone_);
    // get current ds3231 time
    struct tm getDs3231Time();
    // get current ds3231 time as a char pointer
    char *getDs3231TimeAsString();
    // check difference between esp time and ds3231 time --> esp-ds3231
    time_t getTimeDifference();
    // check time difference, synchronize all if difference larger than argument in s
    esp_err_t synchTimeIfDiffLargerThan(time_t allowedTimeDiff_, nixie_time_master_t master_);
    // create a Task that polls clock difference every x seconds and sets synchronizes times if difference larger tan argment
    esp_err_t createSynchTask(time_t synchTaskAllowedTimeDiff_, nixie_time_master_t synchTaskMaster_, TickType_t synchTaskticksToDelay_, BaseType_t xCoreID_);   
    // get and log esp and ds3231 times in terminal
    void logTimes();
    // set local time offset to gmt time
    esp_err_t setLocalTimeOffset(int offset_);
};

#endif /* __NIXIE_TIME_H__ */