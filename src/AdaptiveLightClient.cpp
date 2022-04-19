#include <WiFi.h>
#include "AdaptiveLightClient.h"
#include "Arduino.h"


///////////////

//const char* ssid = "ViktoriPhone";
//const char* password =  "Helena90";
const char* ssid = "Thomas";
const char* password =  "internet";
 
const uint16_t port = 5013;
//const char * host = "172.20.10.3";
const char * host = "192.168.43.160";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;



void configureWiFi(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}


String javaServerRequest(int i,String msg){
    WiFiClient client;
     String line = "Standard msg";
     
    if (!client.connect(host, port)) {  
 
        Serial.println("Connection to host failed");
 
        delay(1000);
        return "";
    }
 
    if(i ==1){
        client.write(msg.c_str(),msg.length());
    }

    if(i==2){
     line = client.readStringUntil('\n');
    }
   
    return line;

}

