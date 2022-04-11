#include <WiFi.h>
#include "AdaptiveLightClient.h"

const char* ssid = "Thomas";
const char* password =  "internet";
 
const uint16_t port = 5013;
const char * host = "192.168.43.160";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;



void configureWiFi(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void javaServerRequest(){
    WiFiClient client;
     
    if (!client.connect(host, port)) {  
 
        Serial.println("Connection to host failed");
 
        delay(1000);
        return;
    }

    client.write("HEJ");
    client.flush();

    String line = client.readString();
    Serial.println(line);

}


