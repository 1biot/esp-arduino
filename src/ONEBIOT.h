#ifndef ONEBIOT_H
#define ONEBIOT_H

#include <time.h>

#ifdef ARDUINO_ARCH_ESP32
#include <WebServer.h>
#elif defined(ARDUINO_ARCH_ESP8266) 
#include <ESP8266WebServer.h>
#endif

#include "utils/config/ONEBIOTConfig.h"
#include "utils/request/ONEBIOTRequestHandler.h"

void ONEBIOT_SERIAL_HEADER_PRINT();

class ONEBIOTApp {
    private:
        ONEBIOTConfig &_config;
        time_t INITIALIZE_TIMESTAMP = 1000000000;
        bool _spiffsStarted = false;
        bool _wifiStarted = false;
        bool _apStarted = false;
        bool _dnsStarted = false;
        bool _webServerStarted = false;
        bool _establishWiFiConnection = false;
        bool _establishWiFiAp = false;
        bool _establishMDNS = false;
        bool _establishWebServer = false;
        bool _updateTime = false;
        time_t _timestamp;
    public:
        ONEBIOTApp(ONEBIOTConfig &config);
        ONEBIOTConfig getConfig();
        bool mountFS();
        void establishWiFiConnection(bool establishWiFiConnection);
        bool couldEstablishWiFiConnection();
        void establishWiFiAP(bool establishWiFiAp);
        bool couldEstablishWiFiAP();
        void establishMDNS(bool establishMDNS);
        bool couldEstablishMDNS();
        void start(bool enforceRestartWhenErrorOccured);
        bool startWiFi();
        void reconnectWiFi();
        bool startAP();
        bool startMDNS();
        bool startMDNS(String hostName);
        void addRequestHandler(ONEBIOTRequestHandler *handler);
        void addServeStatic(const char* uri);
        void initializeTime(int timezone, int daylightOffset_sec, const char* server1, const char* server2);
        time_t updateTime();
        void loop();
        bool isSpiffsStarted();
        bool isWifiStarted();
        bool isApStarted();
        bool isDnsStarted();
        bool isWebserverStarted();
        void restart();
};

#endif //ONEBIOT_H