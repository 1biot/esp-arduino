#ifndef ONEBIOT_CONFIG_H
#define ONEBIOT_CONFIG_H

#include <ArduinoJson.h>

struct ONEBIOTConfigAppConfig {
    String credentials_user;
    String credentials_password;
    String client_name;
    String wifi_ssid;
    String wifi_password;
    bool wifi_establish = false;
    String ap_ssid;
    String ap_password;    
    bool ap_establish = false;
    String dns_name;
    bool dns_establish = false;
};

class ONEBIOTConfig {
    public:
        ONEBIOTConfig(ONEBIOTConfigAppConfig &config, String configFile);
        ONEBIOTConfig(ONEBIOTConfigAppConfig &config, const char *configFile);
        ONEBIOTConfig(ONEBIOTConfigAppConfig &config);
        ONEBIOTConfigAppConfig getConfig();
        String getClientName();
        String getWiFiSsid();
        String getApSsid();
        String getDnsName();
        bool configExists();

        bool setCredentialsUser(String credentialsUser);
        bool setCredentialsPassword(String credentialsPassword);
        bool setClientName(String clientName);

        bool setWiFiSsid(String wifiSsid);
        bool setWiFiPassword(String wifiPassword);
        bool setWiFiEstablish(bool wifiEstablish);

        bool setApSsid(String apSsid);
        bool setApPassword(String apPassword);
        bool setApEstablish(bool apEstablish);

        bool setDnsName(String dnsName);
        bool setDnsEstablish(bool dnsEstablish);
        
        String getConfigFileName();
        bool load();
        void save();
    protected:
        void configToJson(JsonDocument& root);
        void jsonToConfig(JsonDocument& root);
    private:
        ONEBIOTConfigAppConfig &_config;
        String _configFile;
};

#endif //ONEBIOT_CONFIG_H