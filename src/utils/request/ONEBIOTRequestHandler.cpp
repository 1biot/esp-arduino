#ifndef ONEBIOT_REQUEST_CPP
#define ONEBIOT_REQUEST_CPP

#include <utils/request/ONEBIOTRequestHandler.h>
#include <ESP8266WebServer.h>

__attribute__((weak)) String processor(String &key){return key;}

ONEBIOTRequestHandler::ONEBIOTRequestHandler(ONEBIOTConfig config) : _config(config) {}

bool ONEBIOTRequestHandler::canHandle(HTTPMethod method, String uri) {
    return false;
}

bool ONEBIOTRequestHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
    return false;
}

bool ONEBIOTRequestHandler::_authenticate(ESP8266WebServer& server) {
    return server.authenticate(_config.getConfig().credentials_user.c_str(), _config.getConfig().credentials_password.c_str());
}

void ONEBIOTRequestHandler::_sendUnauthorizeResponse(ESP8266WebServer& server) {
    server.requestAuthentication(BASIC_AUTH);
}

bool ONEBIOTRequestHandler::_sendAsTemplate(String fileName, String contentType, ESP8266WebServer &server) {
    // Open file.
    if(!SPIFFS.exists(fileName)) {
        // callback fuckup
        return false;
    }

    File file = SPIFFS.open(fileName, "r");
    if (!file) {
        // callback fuckup
        return false;
    }

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.sendHeader("Content-Type", contentType, true);
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200);

    // Process!
    static const uint16_t MAX = 100;
    String buffer;
    int bufferLen = 0;
    String keyBuffer;
    int val;
    char ch;
    while ((val = file.read()) != -1) {
        ch = char(val);
    
        // Lookup expansion.
        if (ch == _bookend) {
            // Clear out buffer.
            server.sendContent(buffer);
            buffer = "";
            bufferLen = 0;

            // Process substitution.
            keyBuffer = "";
            bool found = false;
            while (!found && (val = file.read()) != -1) {
                ch = char(val);
                if (ch == _bookend) {
                    found = true;
                } else {
                    keyBuffer += ch;
                }
            }
            
            // Check for bad exit.
            if (val == -1 && !found) {
                // callback fuckup
                return false;
            }

            // Get substitution
            String processed = processor(keyBuffer);
            server.sendContent(processed);
        } else {
            bufferLen++;
            buffer += ch;
            if (bufferLen >= MAX) {
                server.sendContent(buffer);
                bufferLen = 0;
                buffer = "";
            }
        }
    }

    if (val == -1) {
        server.sendContent(buffer);
        server.sendContent("");
        return true;
    }
    return false;
}

void ONEBIOTRequestHandler::reset() {
    Serial.println("[WS] restarting the ESP ...");
    delay(2000);
    ESP.reset();
}

#endif //ONEBIOT_REQUEST_CPP