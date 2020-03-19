#ifndef ONEBIOT_CPP
#define ONEBIOT_CPP

#include <Arduino.h>
#include <FS.h>
#include <time.h>

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <mDNS.h>
#elif defined(ARDUINO_ARCH_ESP8266) 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif

#include "ONEBIOT.h"
#include "utils/config/ONEBIOTConfig.h"

WiFiClient ONEBIOTWiFiClient;
#ifdef ARDUINO_ARCH_ESP32
WebServer server(80);
#elif defined(ARDUINO_ARCH_ESP8266) 
ESP8266WebServer server(80);
#endif

// Callbacks definition

__attribute__((weak)) void onMountFS(){}
__attribute__((weak)) void onLoadSettings(String fileName){}
__attribute__((weak)) void onLoadSettingsFailed(String fileName){}
__attribute__((weak)) void onWiFiBegin(){}
__attribute__((weak)) void onWiFiFailed(String message){}
__attribute__((weak)) void onAPBegin(){}
__attribute__((weak)) void onAPFailed(String message){}
__attribute__((weak)) void onDNSBegin(){}
__attribute__((weak)) void onDNSFailed(){}
__attribute__((weak)) void onInitializeTime(time_t timestamp){}
__attribute__((weak)) void onRestart(){}

// print header with help
void ONEBIOT_SERIAL_HEADER_PRINT() {
    Serial.println("");
    Serial.println("##################################");
    Serial.println("### Onebiot ESP8266 - Settings ### ");
    Serial.println("##################################");
    Serial.println("");
    Serial.println("# [OBI] - One Box of IOT");
    Serial.println("# [FIS] - File System");
    Serial.println("# [CNF] - Loading settings");
    Serial.println("# [WFC] - WiFi Connection");
    Serial.println("# [WAP] - AP Connection");
    Serial.println("# [DNS] - mDNS service");
    Serial.println("");
    Serial.println("##################################");
    Serial.println("");
}

// Class definition

ONEBIOTApp::ONEBIOTApp(ONEBIOTConfig &config) : _config(config) {}

ONEBIOTConfig ONEBIOTApp::getConfig() {
    return _config;
}

bool ONEBIOTApp::mountFS() {
    _spiffsStarted = SPIFFS.begin();    
    return _spiffsStarted;
}

bool ONEBIOTApp::couldEstablishWiFiConnection() {
    return _config.getConfig().wifi_establish;
}

bool ONEBIOTApp::couldEstablishWiFiAP() {
    return _config.getConfig().ap_establish;
}

bool ONEBIOTApp::couldEstablishMDNS() {
    return _config.getConfig().dns_establish;
}

void ONEBIOTApp::start(bool enforceRestartWhenErrorOccured) {
    if (!_spiffsStarted && !mountFS() && enforceRestartWhenErrorOccured) {
        restart();
    } else if (_spiffsStarted) {
        onMountFS();
    }

    if (_config.configExists()) {
        if (_config.load()) {
            onLoadSettings(_config.getConfigFileName());
        } else {
            onLoadSettingsFailed(_config.getConfigFileName());
        }
    }
    
    if (couldEstablishWiFiConnection()) {
        if (!startWiFi() && !_establishWiFiAp && !couldEstablishWiFiAP() && enforceRestartWhenErrorOccured) {
            if (enforceRestartWhenErrorOccured) {
                restart();
            }
        }
    }

    if (!_wifiStarted && couldEstablishWiFiAP()) {
        if (!startAP() && enforceRestartWhenErrorOccured) {
            restart();
        }
    }

    if (couldEstablishMDNS()) {
        if (!startMDNS() && enforceRestartWhenErrorOccured) {
            restart();
        }
    }

    if (_establishWebServer) {
        server.onNotFound([](){
            server.send(404, "text/plain", "The content you are looking for was not found.");
        });
        server.begin();
        _webServerStarted = true;
    }
}

bool ONEBIOTApp::startWiFi() {
    if (!couldEstablishWiFiConnection()) {
        onWiFiFailed("WiFi is off");
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);

    if (_config.getConfig().wifi_ssid.isEmpty()) {
        onWiFiFailed("No SSID is available");
        _wifiStarted = false;
        return _wifiStarted;
    }

    if (_config.getConfig().wifi_password.isEmpty()) {
        WiFi.begin(_config.getConfig().wifi_ssid);
    } else {
        WiFi.begin(_config.getConfig().wifi_ssid, _config.getConfig().wifi_password);
    }

    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        onWiFiFailed("Connecting error: #" + String(WiFi.status()));
        _wifiStarted = false;
        return _wifiStarted;
    }

    onWiFiBegin();
    _wifiStarted = true;
    return _wifiStarted;
}

void ONEBIOTApp::reconnectWiFi() {
    if (isWifiStarted() && WiFi.status() != WL_CONNECTED) {
        startWiFi();
    }
}

bool ONEBIOTApp::startAP() {
    if (!couldEstablishWiFiAP()) {
        onAPFailed("Creating AP is off");
        _apStarted = false;
        return _apStarted;
    }

    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
    }

    WiFi.mode(WIFI_AP_STA);
    bool result = WiFi.softAP(_config.getApSsid(), _config.getConfig().ap_password);
    if (!result) {
        onAPFailed("Creating AP failed");
    } else {
        onAPBegin();
    }

    _apStarted = result;
    return _apStarted;
}

bool ONEBIOTApp::startMDNS(String hostName) {
    if (!MDNS.begin(hostName)) {
        onDNSFailed();
        _dnsStarted = false;
        return _dnsStarted;
    }
    MDNS.addService("http", "tcp", 80);

    onDNSBegin();
    _dnsStarted = true;
    return _dnsStarted;
}

bool ONEBIOTApp::startMDNS() {
    return startMDNS(_config.getDnsName());
}

void ONEBIOTApp::addRequestHandler(ONEBIOTRequestHandler *handler) {
    if (couldEstablishWiFiConnection() || couldEstablishWiFiAP()) {
        server.addHandler(handler);
        if (!_establishWebServer) {
            _establishWebServer = true;
        }
    }
}

void ONEBIOTApp::addServeStatic(const char* uri) {
    if (couldEstablishWiFiConnection() || couldEstablishWiFiAP()) {
        server.serveStatic(uri, SPIFFS, uri);
        if (!_establishWebServer) {
            _establishWebServer = true;
        }
    }
}

void ONEBIOTApp::initializeTime(int timezone, int daylightOffset_sec, const char* server1, const char* server2) {
    configTime(timezone, daylightOffset_sec, server1, server2);
    while (_timestamp < INITIALIZE_TIMESTAMP) {
        _timestamp = time(nullptr);
        delay(500);
    }
    onInitializeTime(_timestamp);
}

time_t ONEBIOTApp::updateTime() {
    time(&_timestamp);
    return _timestamp;
}

void ONEBIOTApp::loop() {
    if (couldEstablishWiFiConnection()) {
        reconnectWiFi();
    }

    if (_webServerStarted) {
        server.handleClient();
    }

    if (_dnsStarted) {
        MDNS.update();
    }
}

bool ONEBIOTApp::isSpiffsStarted() {
    return _spiffsStarted;
}

bool ONEBIOTApp::isWifiStarted() {
    return _wifiStarted;
}

bool ONEBIOTApp::isApStarted() {
    return _apStarted;
}

bool ONEBIOTApp::isDnsStarted() {
    return _dnsStarted;
}

bool ONEBIOTApp::isWebserverStarted() {
    return _webServerStarted;
}

void ONEBIOTApp::restart() {
    onRestart();
    delay(100);
    ESP.restart();
}

#endif //ONEBIOT_CPP
