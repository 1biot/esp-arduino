#include <Arduino.h>
#include <time.h>

#include <ONEBIOT.h>
#include <utils/config/ONEBIOTConfig.h>

int timezone = 1;
int dst = 0;
time_t timestamp;

ONEBIOTConfigAppConfig config;
ONEBIOTConfig obiConfig(config);
ONEBIOTApp obiApp(obiConfig);

void onInitializeTime(time_t timestamp) {
    Serial.println("[OBI] Actual date and time is " + String(ctime(&timestamp)));
}

void updateTime() {
    struct tm * timeinfo;
    timestamp = obiApp.updateTime();
    timeinfo = localtime(&timestamp);

    int year = timeinfo->tm_year + 1900;
    int month = timeinfo->tm_mon;
    int day = timeinfo->tm_mday;
    int hour = timeinfo->tm_hour;
    int mins = timeinfo->tm_min;
    int sec = timeinfo->tm_sec;
    int day_of_week = timeinfo->tm_wday;

    Serial.println("[TIME] Y: " + String(year));
    Serial.println("[TIME] m: " + String(month));
    Serial.println("[TIME] d: " + String(day));
    Serial.println("[TIME] H: " + String(hour));
    Serial.println("[TIME] i: " + String(mins));
    Serial.println("[TIME] s: " + String(sec));
    Serial.println("[TIME] DoW: " + String(day_of_week));
}

void setup() {
    Serial.begin(115200);
    ONEBIOT_SERIAL_HEADER_PRINT();

    obiConfig.setWiFiSsid("YOUR_SSID");
    obiConfig.setWiFiPassword("YOUR_PASSWORD");
    obiConfig.setWiFiEstablish(true);

    obiApp.start(true);
    if (obiApp.isWifiStarted()) {
        obiApp.initializeTime(timestamp, timezone * 3600, dst * 0, "cz.pool.ntp.org", "pool.ntp.org");
    }
}

void loop() {
    obiApp.loop();
    updateTime();
}
