#include <SparkFunBQ27441.h>
#include <Wire.h>
#include <WiFi.h>
#include "time.h"



#define SDA_2 33
#define SCL_2 32

const char* ssid = "ViktoriPhone";
const char* password =  "Helena90";
 
const uint16_t port = 5013;
const char * host = "172.20.10.2";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;


// Set BATTERY_CAPACITY to the design capacity of your battery.
const unsigned int BATTERY_CAPACITY = 8000; // e.g. 850mAh battery

BQ27441 lipo_1; 
BQ27441 lipo_2; 
unsigned int soc2;


void configureWiFi(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
}

void setupBQ27441(void)
{
  // Use lipo.begin() to initialize the BQ27441-G1A and confirm that it's
  // connected and communicating.
 /*
  Wire1.begin(SDA_2,SCL_2);
  
  if (!lipo_1.begin(&Wire1))
  {
    Serial.println("Error: Unable to communicate with BQ27441.");
    Serial.println("  Check wiring and try again.");
    Serial.println("  (Battery must be plugged into Battery Babysitter!)");
    while (1) ;
  }
  Serial.println("Connected to first BQ27441!");
  
  */

  Wire.begin();
  
  if (!lipo_2.begin(&Wire))
  {
    Serial.println("Error: Unable to communicate with BQ27441.");
    Serial.println("  Check wiring and try again.");
    Serial.println("  (Battery must be plugged into Battery Babysitter!)");
    while (1) ;
  }
  Serial.println("Connected to second BQ27441!");
  
  // Uset lipo.setCapacity(BATTERY_CAPACITY) to set the design capacity
  // of your battery.
  //lipo_1.setCapacity(BATTERY_CAPACITY);
  lipo_2.setCapacity(BATTERY_CAPACITY);
  
}

void printBatteryStats()
{
  // Read battery stats from the BQ27441-G1A
  //unsigned int soc = lipo_1.soc();  // Read state-of-charge (%)
   soc2 = lipo_2.soc();  // Read state-of-charge (%)
   int current = lipo_2.current(AVG); // Read average current (mA)
   unsigned int fullCapacity = lipo_2.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo_2.capacity(REMAIN); // Read remaining capacity (mAh)
  /*
  unsigned int volts = lipo.voltage(); // Read battery voltage (mV)
  int current = lipo.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo.capacity(REMAIN); // Read remaining capacity (mAh)
  int power = lipo.power(); // Read average power draw (mW)
  int health = lipo.soh(); // Read state-of-health (%)
  */

  // Now print out those values:
  String toPrint = "First Battery " + String(soc2) + "% | ";
 //  toPrint += "Second battery " + String(soc2) + " % | ";
   toPrint += String(current) + " mA | ";
   toPrint += String(capacity) + " / ";
  toPrint += String(fullCapacity) + " mAh | ";

 /*
  toPrint += String(volts) + " mV | ";
 
  toPrint += String(capacity) + " / ";
  toPrint += String(fullCapacity) + " mAh | ";
  toPrint += String(power) + " mW | ";
  toPrint += String(health) + "%";
  */
  
  Serial.println(toPrint);
}

void setup()
{
  Serial.begin(115200);
   pinMode(13, OUTPUT);
  digitalWrite(13,HIGH);
  delay(5000);
  //setupBQ27441();
  configureWiFi();



}
void turnOffCharge(){
  digitalWrite(13,!digitalRead(13));

}
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void loop() 
{

  printLocalTime();
  /*
   WiFiClient client;
     
    if (!client.connect(host, port)) {  
 
        Serial.println("Connection to host failed");
 
        delay(1000);
        return;
    }

    String line = client.readString();

*/

  /*
    if (line.equals("Toggle"))
   {
    turnOffCharge();
   }
   */

   //printBatteryStats();

   delay(1000);
}

