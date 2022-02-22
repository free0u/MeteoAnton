#ifndef WEB_API_H
#define WEB_API_H

#define WEBSERVER_H
#include "ESPAsyncWebServer.h"

class WebApi {
   private:
    bool isInit = false;
    AsyncWebServer* server;

   public:
    WebApi() {}

    void init() {
        if (isInit) {
            return;
        }
        server = new AsyncWebServer(80);
        isInit = true;
    }

    void on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest) {
        server->on(uri, method, onRequest);
    }

    void begin() { server->begin(); }
};

#endif