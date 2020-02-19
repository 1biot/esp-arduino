#ifndef ONEBIOT_CONFIG_CPP
#define ONEBIOT_CONFIG_CPP

#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266) 
#include <ESP8266WiFi.h>
#endif

#include "utils/config/ONEBIOTConfig.h"

const char *DEFAULT_AP_SSID = "ONEBIOT.local";
const char *ONEBIOT_DEFAULT_WS_NAME = "onebiot";
const int JSON_SETTINGS_BUFFER_SIZE = 512;

ONEBIOTConfig::ONEBIOTConfig(ONEBIOTConfigAppConfig &config, String configFile) : _config(config), _configFile(configFile) {}
ONEBIOTConfig::ONEBIOTConfig(ONEBIOTConfigAppConfig &config) : _config(config), _configFile("") {}

ONEBIOTConfigAppConfig ONEBIOTConfig::getConfig() {
    return _config;
}

bool ONEBIOTConfig::configExists() {
    return SPIFFS.exists(_configFile);
}

String ONEBIOTConfig::getClientName() {
    if (_config.client_name.isEmpty()) {
        // 1bio-12:34:56:78:90-12-c1
        String tempClientName = "1biot-" + WiFi.macAddress() + "-" + String(micros() & 0xff, 16);
        return tempClientName;
    }
    return _config.client_name;
}

String ONEBIOTConfig::getDnsName() {
    if (_config.dns_name.isEmpty()) {
        return String(ONEBIOT_DEFAULT_WS_NAME);
    }

    return _config.dns_name;
}

String ONEBIOTConfig::getWiFiSsid() {
    return _config.wifi_ssid;
}

String ONEBIOTConfig::getApSsid() {
    if (_config.ap_ssid.isEmpty()) {
        return String(DEFAULT_AP_SSID);
    }
    return _config.ap_ssid;
}

bool ONEBIOTConfig::setClientName(String clientName) {
    if (clientName != _config.client_name) {
        _config.client_name = clientName;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setCredentialsUser(String credentialsUser) {
    if (credentialsUser != _config.credentials_user) {
        _config.credentials_user = credentialsUser;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setCredentialsPassword(String credentialsPassword) {
    if (String(credentialsPassword) != String(_config.credentials_password)) {
        _config.credentials_password = credentialsPassword;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setApSsid(String apSsid) {
    if (apSsid != _config.ap_ssid) {
        _config.ap_ssid = apSsid;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setApPassword(String apPassword) {
    if (apPassword != String(_config.ap_password)) {
        _config.ap_password = apPassword;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setApEstablish(bool apEstablish) {
    if (apEstablish != _config.ap_establish) {
        _config.ap_establish = apEstablish;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setWiFiSsid(String wifiSsid) {
    if (wifiSsid != _config.wifi_ssid) {
        _config.wifi_ssid = wifiSsid;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setWiFiPassword(String wifiPassword) {
    if (wifiPassword != _config.wifi_password) {
        _config.wifi_password = wifiPassword;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setWiFiEstablish(bool wifiEstablish) {
    if (wifiEstablish != _config.wifi_establish) {
        _config.wifi_establish = wifiEstablish;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setDnsName(String dnsName) {
    if (dnsName != _config.dns_name) {
        _config.dns_name = dnsName;
        return true;
    }
    return false;
}

bool ONEBIOTConfig::setDnsEstablish(bool dnsEstablish) {
    if (dnsEstablish != _config.dns_establish) {
        _config.dns_establish = dnsEstablish;
        return true;
    }
    return false;
}

String ONEBIOTConfig::getConfigFileName() {
    return _configFile;
}

bool ONEBIOTConfig::load() {
    if (!SPIFFS.exists(_configFile)) {
        return false;
    }

    File configFile = SPIFFS.open(_configFile, "r");
    if (!configFile) {
        return false;
    }

    size_t size = configFile.size();

    std::unique_ptr<char[]> buf (new char[size]);
    configFile.readBytes(buf.get(), size);
    
    StaticJsonBuffer<JSON_SETTINGS_BUFFER_SIZE> jsonBuffer;
    JsonObject& doc = jsonBuffer.parseObject(buf.get());
    jsonToConfig(doc);

    configFile.close();
    return true;
}

void ONEBIOTConfig::save() {
    StaticJsonBuffer<JSON_SETTINGS_BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    configToJson(root);
    
    if (SPIFFS.exists(_configFile)) {
        SPIFFS.rename(_configFile, _configFile + ".bak");
    }

    File configFile = SPIFFS.open(_configFile, "w");
    root.printTo(configFile);
    configFile.close();

    if (SPIFFS.exists(_configFile + ".bak")) {
        SPIFFS.remove(_configFile + ".bak");
    }
}

void ONEBIOTConfig::configToJson(JsonObject& root) {
    if (!_config.credentials_user.isEmpty()) {
        root["credentials_user"] = _config.credentials_user;
    }
    
    if (!_config.credentials_password.isEmpty()) {
        root["credentials_password"] = _config.credentials_password;
    }

    if (!_config.client_name.isEmpty()) {
        root["client_name"] = _config.client_name;
    } else {
        root["client_name"] = "";    
    }

    root["wifi_ssid"] = _config.wifi_ssid;
    root["wifi_password"] = _config.wifi_password;
    root["wifi_establish"] = _config.wifi_establish ? 1 : 0;
    
    root["ap_ssid"] = _config.ap_ssid;
    root["ap_password"] = _config.ap_password;
    root["ap_establish"] = _config.ap_establish ? 1 : 0;
    
    root["dns_name"] = _config.dns_name;
    root["dns_establish"] = _config.dns_establish ? 1 : 0;
}

void ONEBIOTConfig::jsonToConfig(JsonObject& root) {
    _config.credentials_user = root["credentials_user"] | "";
    _config.credentials_password = root["credentials_password"] | "";
    _config.client_name = root["client_name"] | "";
    
    _config.wifi_ssid = root["wifi_ssid"] | "";
    _config.wifi_password = root["wifi_password"] | "";

    bool wifiEstablish = root["wifi_establish"] == "1" ? true : false;
    _config.wifi_establish = wifiEstablish;

    _config.ap_ssid = root["ap_ssid"] | "";
    _config.ap_password = root["ap_password"] | "";
    
    bool apEstablish = root["ap_establish"] == "1" ? true : false;
    _config.ap_establish = apEstablish;
    
    _config.dns_name = root["dns_name"] | "";
    
    bool dnsEstablish = root["dns_establish"] == "1" ? true : false;
    _config.dns_establish = dnsEstablish;
}
#endif //ONEBIOT_CONFIG_CPP