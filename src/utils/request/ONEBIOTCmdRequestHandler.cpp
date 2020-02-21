#ifndef CMD_REQUEST_CPP
#define CMD_REQUEST_CPP

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "utils/request/ONEBIOTCmdRequestHandler.h"
#include "utils/request/ONEBIOTRequestHandler.h"
#include "utils/config/ONEBIOTConfig.h"

const char *SECURE_VALUE = "<secure_value>";
const char *UNKNOWN_VALUE = "<unknown_value>";

const char *CMD_CREDENTIALS = "/cmd/credentials";
const char *CMD_WIFI = "/cmd/wifi";
const char *CMD_WIFI_LIST = "/cmd/wifi/list";
const char *CMD_AP = "/cmd/ap";
const char *CMD_DNS = "/cmd/dns";
const char *CMD_STATS = "/cmd/stats";
const char *CMD_STATS_ESP = "/cmd/stats/esp";
const char *CMD_STATS_SPIFFS = "/cmd/stats/spiffs";
const char *CMD_OPTION = "/cmd/option/";
const char *CMD_RESET = "/cmd/reset";

__attribute__((weak)) void onNeedRestart(){}

bool ONEBIOTCmdRequestHandler::canHandle(HTTPMethod method, String uri) {
    if (uri == CMD_WIFI_LIST && method == HTTP_GET) {
        return true;
    } else if (uri == CMD_STATS && method == HTTP_GET) {
        return true;
    } else if (uri == CMD_STATS_ESP && method == HTTP_GET) {
        return true;
    } else if (uri == CMD_STATS_SPIFFS && method == HTTP_GET) {
        return true;
    } else if (uri == CMD_CREDENTIALS && method == HTTP_POST) {
        return true;
    } else if (uri == CMD_RESET && method == HTTP_POST) {
        return true;
    } else if (uri == CMD_WIFI && (method == HTTP_GET || method == HTTP_POST)) {
        return true;
    } else if (uri == CMD_AP && (method == HTTP_GET || method == HTTP_POST)) {
        return true;
    } else if (uri == CMD_DNS && (method == HTTP_GET || method == HTTP_POST)) {
        return true;
    }

    if (uri.indexOf("/cmd/option/") == 0 && (method == HTTP_GET)) {
        _optionParam = uri.substring(12);
        if (_optionParam.isEmpty()) {
            return false;
        }
        return true;
    }
    return false;
}

bool ONEBIOTCmdRequestHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
    if (!ONEBIOTRequestHandler::_authenticate(server)) {
        ONEBIOTRequestHandler::_sendUnauthorizeResponse(server);
        return true;
    }

    bool needRestart = false;
    __payload = String("");
    DynamicJsonDocument response(2048);

    if (requestUri == CMD_WIFI_LIST && requestMethod == HTTP_GET) {
        CMD_WIFI_LIST_CALLBACK(response, server, requestMethod);
    } else if (requestUri == CMD_STATS && requestMethod == HTTP_GET) {
        CMD_STATS_CALLBACK(response);
    } else if (requestUri == CMD_STATS_ESP && requestMethod == HTTP_GET) {
        CMD_STATS_ESP_CALLBACK(response);
    } else if (requestUri == CMD_STATS_SPIFFS && requestMethod == HTTP_GET) {
        CMD_STATS_SPIFFS_CALLBACK(response);
    } else if (requestUri == CMD_CREDENTIALS && requestMethod == HTTP_POST) {
        CMD_CREDENTIALS_CALLBACK(response, server, requestMethod);
    }  else if (requestUri == CMD_RESET && requestMethod == HTTP_POST) {
        CMD_RESET_CALLBACK(response);
        needRestart = true;
    } else if (requestUri == CMD_WIFI && (requestMethod == HTTP_GET || requestMethod == HTTP_POST)) {
        CMD_WIFI_CALLBACK(response, server, requestMethod);
    } else if (requestUri == CMD_AP && (requestMethod == HTTP_GET || requestMethod == HTTP_POST)) {
        CMD_AP_CALLBACK(response, server, requestMethod);
    } else if (requestUri == CMD_DNS && (requestMethod == HTTP_GET || requestMethod == HTTP_POST)) {
        CMD_DNS_CALLBACK(response, server, requestMethod);
    } else if (requestUri.indexOf("/cmd/option/") == 0 && (requestMethod == HTTP_GET)) {
        CMD_OPTION_CALLBACK(response);
    }

    if (response.size()) {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        serializeJson(response, __payload);
        server.send(200, "application/json", __payload);    
        if (needRestart) {
            onNeedRestart();
        }
        return true;
    }

    return false;
}

bool ONEBIOTCmdRequestHandler::CMD_RESET_CALLBACK(JsonDocument& response) {
    response["success"] = true;
    response["message"] = "Device restarting";
    return true;
}

bool ONEBIOTCmdRequestHandler::CMD_CREDENTIALS_CALLBACK(JsonDocument& response, ESP8266WebServer& server, HTTPMethod requestMethod) {
    if (requestMethod != HTTP_POST) {
        response["success"] = false;
        response["message"] = "Invalid request.";
    } else if (server.arg("credentials_user").isEmpty() || server.arg("credentials_password").isEmpty()) {
        response["success"] = false;
        response["message"] = "User and password are empty. Operation is not allowed.";
    } else {
        bool changedUser = _config.setCredentialsUser(server.arg("credentials_user"));
        bool changedPassword = _config.setCredentialsUser(server.arg("credentials_password"));
        if (changedUser || changedPassword) {
            _config.save();
        }
        
        response["success"] = true;
        response["message"] = "Credentials has been changed.";
    }
    return true;
}

bool ONEBIOTCmdRequestHandler::CMD_WIFI_LIST_CALLBACK(JsonDocument& response, ESP8266WebServer& server, HTTPMethod requestMethod) {
    if (requestMethod == HTTP_GET) {
        int scanCount = WiFi.scanNetworks();
        if (scanCount == WIFI_SCAN_RUNNING) {
            response["success"] = false;
            response["message"] = "Scanning...";
        } else if (scanCount == WIFI_SCAN_FAILED) {
            response["success"] = false;
            response["message"] = "Scanning failed.";
        } else if (scanCount) {
            response["success"] = true;

            JsonArray data = response.createNestedArray("data");
            for (int i = 0; i < 5; ++i) { // max 5 wifis :-)
                JsonObject network = data.createNestedObject();
                network["ssid"] = WiFi.SSID(i);
                network["encryption"] = WiFi.encryptionType(i);
                network["rssi"] = WiFi.RSSI(i);
                network["bssid"] = WiFi.BSSIDstr(i);
                network["channel"] = WiFi.channel(i);
                network["isHidden"] = WiFi.isHidden(i);
            }

            WiFi.scanDelete();
            if (WiFi.scanComplete() == WIFI_SCAN_FAILED) {
                WiFi.scanNetworks(true);
            }
        } else {
            response["success"] = false;
            response["message"] = "No WiFi networks founds.";
        }
        return true;
    }
    return false;
}

bool ONEBIOTCmdRequestHandler::CMD_WIFI_CALLBACK(JsonDocument& response, ESP8266WebServer& server, HTTPMethod requestMethod) {
    if (requestMethod == HTTP_GET) {
        if (WiFi.status() == WL_CONNECTED) {
            response["success"] = true;
            JsonObject data = response.createNestedObject("data");
            data["ssid"] = WiFi.SSID();
            data["rssi"] = WiFi.RSSI();
            data["bssid"] = WiFi.BSSIDstr();
            data["channel"] = WiFi.channel();
            data["local_ip"] = WiFi.localIP().toString();
            data["dns_ip"] = WiFi.dnsIP().toString();
            data["gateway_ip"] = WiFi.gatewayIP().toString();
        } else {
            response["success"] = false;
            response["message"] = "ESP is disconnected from the WiFi";
        }

        return true;
    } else if (requestMethod == HTTP_POST) {
        if (server.args() == 0) {
            return false;
        }

        _config.setWiFiSsid(server.arg("wifi_ssid"));
        _config.setWiFiPassword(server.arg("wifi_password"));
        _config.setWiFiEstablish(server.arg("wifi_establish"));
        _config.save();

        response["success"] = true;
        response["message"] = "WiFi settings saved. Please restart the ESP.";
        return true;
    }
    return false;
}

bool ONEBIOTCmdRequestHandler::CMD_AP_CALLBACK(JsonDocument& response, ESP8266WebServer& server, HTTPMethod requestMethod) {
    if (requestMethod == HTTP_GET) {
        if (WiFi.getMode() == WIFI_AP_STA) {
            response["success"] = true;
            JsonObject data = response.createNestedObject("data");
            data["ssid"] = WiFi.softAPSSID();
            data["psk"] = WiFi.softAPPSK();
            data["ip"] = WiFi.softAPIP().toString();
            data["mac_address"] = WiFi.softAPmacAddress();
            data["station_num"] = WiFi.softAPgetStationNum();
        } else {
            response["success"] = false;
            response["message"] = "ESP has disconected AP";
        }
        return true;
    } else if (requestMethod == HTTP_POST) {
        if (server.args() == 0) {
            return false;
        }

        _config.setApSsid(server.arg("ap_ssid"));
        _config.setApPassword(server.arg("ap_password"));
        _config.setApEstablish(server.arg("ap_establish"));
        _config.save();

        response["success"] = true;
        response["message"] = "AP settings saved. Please restart the ESP.";
        return true;
    }
    return false;
}

bool ONEBIOTCmdRequestHandler::CMD_DNS_CALLBACK(JsonDocument& response, ESP8266WebServer& server, HTTPMethod requestMethod) {
    if (requestMethod == HTTP_GET) {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _config.getDnsName();
        data["local_name"] = _config.getDnsName() + String(".local");
        return true;
    } else if (requestMethod == HTTP_POST) {
        if (server.args() == 0) {
            return false;
        }

        _config.setDnsName(server.arg("dns_name"));
        _config.setApEstablish(server.arg("dns_establish"));
        _config.save();

        response["success"] = true;
        response["message"] = "DNS settings saved. Please restart the ESP.";
        return true;
    }
    return false;
}

bool ONEBIOTCmdRequestHandler::CMD_STATS_CALLBACK(JsonDocument& response) {
    response["success"] = true;
    
    FSInfo fs_info;
    SPIFFS.info(fs_info);

    JsonObject data = response.createNestedObject("data");
    data["spiffs_total_bytes"] = fs_info.totalBytes;
    data["spiffs_used_bytes"] = fs_info.usedBytes;
    data["spiffs_block_size"] = fs_info.blockSize;
    data["spiffs_page_size"] = fs_info.pageSize;
    data["spiffs_max_open_files"] = fs_info.maxOpenFiles;
    data["spiffs_max_path_length"] = fs_info.maxPathLength;
    
    data["esp_free_heap"] = ESP.getFreeHeap();
    data["esp_heap_fragmentation"] = ESP.getHeapFragmentation();
    data["esp_max_free_block_size"] = ESP.getMaxFreeBlockSize();
    data["esp_chip_id"] = ESP.getChipId();
    data["esp_core_version"] = ESP.getCoreVersion();
    data["esp_sdk_version"] = ESP.getSdkVersion();
    data["esp_cpu_freq"] = ESP.getCpuFreqMHz();
    data["esp_sketch_size"] = ESP.getSketchSize();
    data["esp_free_sketch_space"] = ESP.getFreeSketchSpace();
    data["esp_sketch_md5"] = ESP.getSketchMD5();
    data["esp_flash_chip_id"] = ESP.getFlashChipId();
    data["esp_flash_chip_size"] = ESP.getFlashChipSize();
    data["esp_flash_chip_real_size"] = ESP.getFlashChipRealSize();

    return true;
}

bool ONEBIOTCmdRequestHandler::CMD_STATS_ESP_CALLBACK(JsonDocument& response) {
    response["success"] = true;

    JsonObject data = response.createNestedObject("data");
    data["esp_free_heap"] = ESP.getFreeHeap();
    data["esp_heap_fragmentation"] = ESP.getHeapFragmentation();
    data["esp_max_free_block_size"] = ESP.getMaxFreeBlockSize();
    data["esp_chip_id"] = ESP.getChipId();
    data["esp_core_version"] = ESP.getCoreVersion();
    data["esp_sdk_version"] = ESP.getSdkVersion();
    data["esp_cpu_freq"] = ESP.getCpuFreqMHz();
    data["esp_sketch_size"] = ESP.getSketchSize();
    data["esp_free_sketch_space"] = ESP.getFreeSketchSpace();
    data["esp_sketch_md5"] = ESP.getSketchMD5();
    data["esp_flash_chip_id"] = ESP.getFlashChipId();
    data["esp_flash_chip_size"] = ESP.getFlashChipSize();
    data["esp_flash_chip_real_size"] = ESP.getFlashChipRealSize();

    return true;
}

bool ONEBIOTCmdRequestHandler::CMD_STATS_SPIFFS_CALLBACK(JsonDocument& response) {
    response["success"] = true;

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    
    JsonObject data = response.createNestedObject("data");
    data["spiffs_total_bytes"] = fs_info.totalBytes;
    data["spiffs_used_bytes"] = fs_info.usedBytes;
    data["spiffs_block_size"] = fs_info.blockSize;
    data["spiffs_page_size"] = fs_info.pageSize;
    data["spiffs_max_open_files"] = fs_info.maxOpenFiles;
    data["spiffs_max_path_length"] = fs_info.maxPathLength;

    return true;
}
bool ONEBIOTCmdRequestHandler::CMD_OPTION_CALLBACK(JsonDocument& response) {
    if (_optionParam == "client_name") {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = _config.getClientName();
    } else if (_optionParam == "credentials_user") {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = SECURE_VALUE;
    } else if (_optionParam == "credentials_password") {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = SECURE_VALUE;
    } else if (_optionParam == "ap_ssid") {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = _config.getConfig().ap_ssid;
    } else if (_optionParam == "ap_password") {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = SECURE_VALUE;
    } else if (_optionParam == "wifi_ssid") {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = _config.getConfig().wifi_ssid;
    } else if (_optionParam == "wifi_password") {
        response["success"] = true;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = SECURE_VALUE;
    } else {
        response["success"] = false;
        JsonObject data = response.createNestedObject("data");
        data["name"] = _optionParam;
        data["value"] = UNKNOWN_VALUE;
    }
    return true;   
}

#endif //CMD_REQUEST_CPP