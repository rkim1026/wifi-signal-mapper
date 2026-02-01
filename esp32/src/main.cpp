#include <Arduino.h>
#include <WiFi.h>
#include "ECE140_WIFI.h"
#include "ECE140_MQTT.h"
#include <vector>
#include <algorithm>
#include <esp_system.h>



constexpr const char* CLIENT_ID = "NETGEAR17";// fill this, this needs to be unique
constexpr const char* TOPIC_PREFIX = "wifiscanner";//fill this, this needs to match topic prefix from the subscriber.py. Make sure it does not end with / 

ECE140_MQTT mqtt(CLIENT_ID, TOPIC_PREFIX);
ECE140_WIFI wifi;

//WIFI setup
const char* ucsdUsername = UCSD_USERNAME;
String ucsdPassword = String(UCSD_PASSWORD);
const char* wifiSsid = WIFI_SSID;
const char* nonEnterpriseWifiPassword = NON_ENTERPRISE_WIFI_PASSWORD;

// --- Timing Variables ---
unsigned long lastPublish = 0;
const long interval = 5000; // 5 seconds in milliseconds

struct NetworkInfo {
    String ssid;
    int32_t rssi;
};

bool compareByRSSI(const NetworkInfo a, const NetworkInfo b) {
    return a.rssi > b.rssi; // Sort in descending order
}


String boardInfo = ESP.getChipModel(); 

void setup() {
    Serial.begin(115200);
    
    //Connection setup
    Serial.println("Setup done. Starting WiFi Scan...");
    if(strlen(nonEnterpriseWifiPassword)<2){
        wifi.connectToWPAEnterprise(wifiSsid, ucsdUsername, ucsdPassword);
        Serial.println("ucsd");
    } else {
        wifi.connectToWiFi(wifiSsid,nonEnterpriseWifiPassword);
        Serial.println("local");
    }
    delay(2000);
    mqtt.connectToBroker();
    mqtt.subscribeTopic("command");

}

void loop() {
    mqtt.loop();

    // here are some hints for using wifi network scanner
    // you can get count using
    // int numNetworks = WiFi.scanNetworks();
    // you can learn about the ith wifi network name using
    // WiFi.SSID(i)
    // you can learn about the ith wifi network signal strength using 
    // WiFi.RSSI(i)
    // you will also probably want to construct a json object

    /*
    {
        "device_id": "esp32-001",
        "timestamp": 12345,
        "connected_ssid": "MyNetwork",
        "connected_rssi": -45,
        "networks": [
            {"ssid": "Network1", "rssi": -45},
            {"ssid": "Network2", "rssi": -62}
        ]
    }
    */
    unsigned long currentMillis = millis();

    std::vector<NetworkInfo> foundNetworks;

    if (currentMillis - lastPublish >= interval) {
        lastPublish = currentMillis;

        int numNetworks = WiFi.scanNetworks();

        for (int i = 0; i < numNetworks; ++i) {
            if (WiFi.RSSI(i) >= -80) {
                foundNetworks.push_back({WiFi.SSID(i), WiFi.RSSI(i)});
            }
        }
        //sorts networks by RSSI
        std::sort(foundNetworks.begin(), foundNetworks.end(), compareByRSSI);

        if (foundNetworks.size() > 10) {
            foundNetworks.resize(10);
        }

        //makes JSON message
        String msg = "{\n";
        msg += "  \"device_id\": \"" + boardInfo + "\",\n";
        msg += "  \"timestamp\": " + String(currentMillis) + ",\n";
        msg += "  \"connected_ssid\": \"" + String(wifiSsid) + "\",\n";
        msg += "  \"connected_rssi\": " + String(WiFi.RSSI()) + ",\n";
        msg += "  \"networks\": [\n";
        
        for (size_t i = 0; i < foundNetworks.size(); ++i) {
            msg += "    {\"ssid\": \"" + foundNetworks[i].ssid + "\", \"rssi\": " + String(foundNetworks[i].rssi) + "}";
            if (i < foundNetworks.size() - 1) {
                msg += ",";
            }
            msg += "\n";
        }
        
        msg += "  ]\n";
        msg += "}";

        //publishes message with topic scan
        mqtt.publishMessage("scan", msg);
        Serial.println("Published WiFi scan data:");
        Serial.println(msg.c_str());

        WiFi.scanDelete();
    }

}


