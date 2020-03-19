#include <Arduino.h>

/**
 * Settings for compiling Scheduler
 * #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
 * #define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
 * #define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
 * #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
 * #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
 * #define _TASK_PRIORITY          // Support for layered scheduling priority
 * #define _TASK_MICRO_RES         // Support for microsecond resolution
 * #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 and ESP32 ONLY)
 * #define _TASK_DEBUG             // Make all methods and variables public for debug purposes
 * #define _TASK_INLINE            // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
 * #define _TASK_TIMEOUT           // Support for overall task timeout
 * #define _TASK_OO_CALLBACKS      // Support for dynamic callback method binding
**/
#define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>
#include <time.h>

#include <ONEBIOT.h>
#include <utils/config/ONEBIOTConfig.h>
#include <utils/request/ONEBIOTCmdRequestHandler.h>

#define DEBUG_SERIAL
#define DEBUG_CONFIG

// ######################## ONEBIOT APP CONFIG ########################
const char *configFile = "/onebiot.json";
const char *configFileBak = "/onebiot.json.bak";

int timezone = 1;
int dst = 0;
time_t timestamp;

ONEBIOTConfigAppConfig config;
ONEBIOTConfig obiConfig(config, configFile);
ONEBIOTApp obiApp(obiConfig);

// ######################## Task scheduling ########################

Scheduler ts;
// Callback methods prototypes
void testTaskCallBack();

//Task testTask(10 * TASK_MINUTE, TASK_FOREVER, &testTaskCallBack, &ts);
Task testTask(30 * TASK_SECOND, TASK_FOREVER, &testTaskCallBack, &ts);
void testTaskCallBack() {
    digitalWrite(LED_BUILTIN, LOW);

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
    
    digitalWrite(LED_BUILTIN, HIGH);
}
// ######################## Application life cycle callbacks ########################
// We using this callbacks as loggers, but we can use it for more complexible usecase
// as we shown at bellow

// when file system is ready
void onMountFS() {
    #ifdef DEBUG_SERIAL
        Serial.println("[FIS] Filesystem (SPIFFS) has been loaded.");
    #endif
}

// Callback occurres when config json file loaded.
// This callback turning 1biotApp into AP or WiFi mode when SSID is empty and allows dns .local name
// Default .local name is onebiot.local
void onLoadSettings(String fileName) {
    #ifdef DEBUG_SERIAL
        Serial.println("[CNF] Configuration from file " + fileName + " has been loaded");
    #endif

    if (obiConfig.getConfig().wifi_ssid.isEmpty()) { // or same user and password as default
        obiConfig.setApEstablish(true);
    } else {
        obiConfig.setWiFiEstablish(true);
    }

    obiConfig.setDnsEstablish(true);
}

// when config json file failed.
void onLoadSettingsFailed(String fileName) {
    #ifdef DEBUG_SERIAL
        Serial.println("[CNF] Loading configuration of " + fileName + " failed!");
    #endif
}

// on WiFi start. We adding system api accessible by http (http://[IP]/cmd/) for more information visit https://onebiot.github.com/CMDRequest
void onWiFiBegin() {
    #ifdef DEBUG_SERIAL
        Serial.println("[WFC] WiFi connected");
        Serial.print("[WFC] IP address: ");
        Serial.println(WiFi.localIP());
    #endif

    // API
    obiApp.addRequestHandler(new ONEBIOTCmdRequestHandler(obiApp.getConfig()));

    // debug dowloading option file
    #ifdef DEBUG_CONFIG
        obiApp.addServeStatic(configFile);
        obiApp.addServeStatic(configFileBak);
    #endif
}

void onWiFiFailed(String message) {
    #ifdef DEBUG_SERIAL
        Serial.println("[WFC] Could not connect to WiFi!");
        if (!message.isEmpty()) {
            Serial.println("[WFC] " + message);
        }
    #endif
}

void onAPBegin () {
    #ifdef DEBUG_SERIAL
        Serial.print("[WAP] SSID: ");
        Serial.println(WiFi.softAPSSID());
        Serial.print("[WAP] IP address: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("[WAP] MAC address: ");
        Serial.println(WiFi.softAPmacAddress());
    #endif

    // API
    obiApp.addRequestHandler(new ONEBIOTCmdRequestHandler(obiApp.getConfig()));

    // WEB GUI
    // under construct
    //obiApp.addRequestHandler(new ONEBIOTIndexRequestHandler(obiApp.getConfig()));
    //obiApp.addServeStatic("/chota.min.css");
    //obiApp.addServeStatic("/main.js");

    // debug dowloading option file
    #ifdef DEBUG_CONFIG
        obiApp.addServeStatic(configFile);
        obiApp.addServeStatic(configFileBak);
    #endif
}

void onAPFailed (String message) {
    #ifdef DEBUG_SERIAL
        Serial.println("[WAP] Could not create an AP!");
        if (!message.isEmpty()) {
            Serial.println("[WAP] " + message);
        }
    #endif
}

void onDNSBegin() {
    #ifdef DEBUG_SERIAL
        Serial.println(String("[DNS] http://") + obiConfig.getDnsName() + String(".local started"));
    #endif
}

void onDNSFailed () {
    #ifdef DEBUG_SERIAL
        Serial.println("[DNS] Error setting up DNS responder");
    #endif
}

void onInitializeTime(time_t timestamp) {
    #ifdef DEBUG_SERIAL
        Serial.println("[OBI] Actual date and time is " + String(ctime(&timestamp)));
    #endif
}

void onNeedRestart() {
    #ifdef DEBUG_SERIAL
        Serial.println("[OBI] Triggered restart from webserver");
    #endif
    obiApp.restart();
}

void onRestart() {
    #ifdef DEBUG_SERIAL
        Serial.println("[OBI] ESP8266 restarting now ...");
    #endif
}

// ######################## MAIN APP LOOP ########################
void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN , OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    ONEBIOT_SERIAL_HEADER_PRINT();

    obiApp.start(true);
    if (obiApp.isWifiStarted()) {
        obiApp.initializeTime(timezone * 3600, dst * 0, "cz.pool.ntp.org", "pool.ntp.org");
        ts.addTask(testTask);
        testTask.enable();
    }
}

void loop() {
    obiApp.loop();
    if (obiApp.isWifiStarted()) {
        ts.execute();
        yield();
    }
}
