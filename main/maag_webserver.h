/*
    ...

*/
#ifndef __MAAG_WEBSERVER_H__
#define __MAAG_WEBSERVER_H__

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/api.h"

// http_serve function type
typedef void (*http_serve_function)(struct netconn *conn);

// Webserver class with a standrad http_serve memeber function. This class is meant to be inherited
// so that a user can create his individual http_serve function and handle his own variables accordingly
//
// Inherite this class and define a user specific http_serve member function. Something like:
//
// 1. create user class NixieWebserver : public MaagWebserver
// 2. define a static void xyz_http_serve(struct netconn *conn) function in user class
// 3. define user parameters used in xyz_http_serve()
// 4. initialize in a main: NixieWebserver webserver; webserver.setHttpServeFunc(webserver.nixie_http_serve); webserver.createServer();
class MaagWebserver
{
private:
    const char *TAG = "maag_webserver_class";

public:
    MaagWebserver(/* args */);
    ~MaagWebserver();

    // set http_server function which will handle http POST/GET requests
    void setHttpServeFunc(http_serve_function pHttpServeFunc);

    // create and start the http server
    void createServer();

    // default http_serve function
    static void default_http_serve(struct netconn *conn);
};

#endif /* __MAAG_WEBSERVER_H__ */