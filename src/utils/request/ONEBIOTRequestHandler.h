#ifndef ONEBIOT_REQUEST_H
#define ONEBIOT_REQUEST_H

#ifdef ARDUINO_ARCH_ESP32
#include <WebServer.h>
#elif defined(ARDUINO_ARCH_ESP8266) 
#include <ESP8266WebServer.h>
#endif

#include "utils/config/ONEBIOTConfig.h"

class ONEBIOTRequestHandler : public RequestHandler {
    public:
        ONEBIOTRequestHandler(ONEBIOTConfig config);
        bool canHandle(HTTPMethod method, String uri) override;
        bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override;
    protected:
        ONEBIOTConfig _config;
        char _bookend = '%';
        bool _authenticate(ESP8266WebServer& server);
        void _sendUnauthorizeResponse(ESP8266WebServer& server);
        bool _sendAsTemplate(String fileName, String contentType, ESP8266WebServer &server);
        void reset();
};

#endif //ONEBIOT_REQUEST_H