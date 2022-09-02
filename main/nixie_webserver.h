/*
    ...

*/
#ifndef __NIXIE_WEBSERVER_H__
#define __NIXIE_WEBSERVER_H__


#include "maag_webserver.h"

// data structure for nixie webserver data
typedef struct{
    uint16_t ui16HttpRequestCounter;
} nixie_webserver_data_t;


// nixie-specific webserver class
class NixieWebserver : public MaagWebserver
{
private:
    // Nixie-specific http_server
    static void nixie_http_serve(struct netconn *conn, void *pArgs);
    // User data for nixie webserver
    nixie_webserver_data_t m_data;

public:
    // set nixie http_server here
    NixieWebserver();

    // get all data
    nixie_webserver_data_t getdata(){
        return m_data;
    }
    // get communication counter
    uint16_t getCommunicationCounter(){
        return m_data.ui16HttpRequestCounter;
    }

};




#endif /* __NIXIE_WEBSERVER_H__ */