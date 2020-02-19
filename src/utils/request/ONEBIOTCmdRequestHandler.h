#ifndef CMD_REQUEST_H
#define CMD_REQUEST_H

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include "utils/config/ONEBIOTConfig.h"
#include "utils/request/ONEBIOTRequestHandler.h"

class ONEBIOTCmdRequestHandler : public ONEBIOTRequestHandler {
    public:
        ONEBIOTCmdRequestHandler(ONEBIOTConfig config) : ONEBIOTRequestHandler(config) {}
        bool canHandle(HTTPMethod method, String uri) override;

        bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override;
    protected:
        bool CMD_RESET_CALLBACK(JsonObject& response);
        bool CMD_WIFI_LIST_CALLBACK(JsonObject& response, ESP8266WebServer& server, HTTPMethod requestMethod);
        bool CMD_WIFI_CALLBACK(JsonObject& response, ESP8266WebServer& server, HTTPMethod requestMethod);
        bool CMD_CREDENTIALS_CALLBACK(JsonObject& response, ESP8266WebServer& server, HTTPMethod requestMethod);
        bool CMD_AP_CALLBACK(JsonObject& response, ESP8266WebServer& server, HTTPMethod requestMethod);
        bool CMD_STATS_CALLBACK(JsonObject& response);
        bool CMD_STATS_ESP_CALLBACK(JsonObject& response);
        bool CMD_STATS_SPIFFS_CALLBACK(JsonObject& response);
        bool CMD_OPTION_CALLBACK(JsonObject& response);
    private:
        String _optionParam;
        String __payload;
};


#endif //CMD_REQUEST_H