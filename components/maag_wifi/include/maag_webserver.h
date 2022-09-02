/*
    ...

*/
#ifndef __MAAG_WEBSERVER_H__
#define __MAAG_WEBSERVER_H__

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "freertos/queue.h"

// http_serve function type. Needed to pass function pointers, not quite sure how this works actually :)
typedef void (*http_serve_function)(struct netconn *conn, void *pArgs);

// Webserver class with a standrad http_serve memeber function. This class is meant to be inherited
// so that a user can create his individual http_serve function and handle his own variables accordingly
//
// Inherite this class and define a user specific http_serve member function. Something like:
//
// 1. create user class NixieWebserver : public MaagWebserver
// 2. define a static void user_http_serve(struct netconn *conn) function in user class, SET this function with setHttpServeFunc(). Example: setHttpServeFunc(default_http_serve); 
// 3. define user arguments (as a typedef), pass this pointer with setHttpServeArgs(). Example: setHttpServeArgs((void *)&iDefaultArg_);
// --> Pro tip: do this in the constructor of inherited class
// 4. initialize in a main. Example: NixieWebserver webserver; webserver.createServer();
class MaagWebserver
{
private:
    // server & handel task, gracefully copied thanks to the internet ;) These have to be static because they are going to be FreeRtos Tasks,
    // which may not be non-static member functions
    static void server_task(void *pArgs);
    static void server_handle_task(void *pArgs);
    // default http_serve function. Will be set as default and must be overwritten by a user-defined, project-specific function
    static void default_http_serve(struct netconn *conn, void *pArgs);
    // variable to store function pointer type
    static http_serve_function httpServeFunc; 
    //static int m_iDefaultArg;
    int m_newDefaultArg = 0;
    // some event queue stuff
    static QueueHandle_t m_client_queue;
    static const int m_client_queue_size;

public:
    MaagWebserver();
    // set http_server function which will handle http POST/GET requests. Pass the desired function pointer
    void setHttpServeFunc(http_serve_function pHttpServeFunc);
    // create and start the http server on the desired Core (0 or 1)
    void createServer(BaseType_t xCoreID);
};

#endif /* __MAAG_WEBSERVER_H__ */