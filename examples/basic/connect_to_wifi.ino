#include <Arduino.h>

#include <ONEBIOT.h>
#include <utils/config/ONEBIOTConfig.h>

ONEBIOTConfigAppConfig config;
ONEBIOTConfig obiConfig(config);
ONEBIOTApp obiApp(obiConfig);

void onWiFiBegin() {
    Serial.println("WiFi connected");
    Serial.println("SSID: " + WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());
}

void onWiFiFailed(String message) {
    Serial.println("Could not connect to WiFi!");
    if (!message.isEmpty()) {
        Serial.println(message);
    }
}

void setup() {
    Serial.begin(115200);
    ONEBIOT_SERIAL_HEADER_PRINT();

    obiConfig.setWiFiSsid("YOUR_SSID");
    obiConfig.setWiFiPassword("YOUR_PASSWORD");
    obiConfig.setWiFiEstablish(true);

    obiApp.start(false);
    // when use a true you didn't use to obiApp.isWifiStarted(). ESP restart when connection is not available
    // obiApp.start(true);
}

void loop() {
    if (obiApp.isWifiStarted()) {
        // do stuff
    }
}
