/*
    ...

*/
#ifndef __MAAG_SNTP_H__
#define __MAAG_SNTP_H__

#include "esp_sntp.h"

class MaagSNTP
{
private:
    // static callback function pointer
    static sntp_sync_time_cb_t m_callback_func;
    // static init counter
    static uint8_t ui8firstInit;
    // desired interval
    uint32_t m_ui32interval_ms = 60000;
    // default, static callback function which just prints system time, if system time has been synchronized via sntp server
    static void defaultSyncNotificationCb(struct timeval *tv);

public:
    MaagSNTP();
    // set synch interval in ms
    void setSynchInterval(uint32_t ui32interval_ms_);
    // set a callback function which is called when time is synched via sntp. Must be type sntp_sync_time_cb_t
    void setSyncNotificationCb(sntp_sync_time_cb_t callback_func_);
    // init sntp server with current settings and start service
    void initStart();
};

#endif /* __MAAG_SNTP_H__ */