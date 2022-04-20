#include <SparkFunBQ27441.h>
#include <Wire.h>
#include <WiFi.h>
#include "time.h"
#include "esp_timer.h"
#include <esp_task_wdt.h>
#include "Arduino.h"
#include "LuxSensor.h"
#include "AdaptiveLightClient.h"

#define CHARGE_RELAY_PIN 33
#define LED_PIN 32
#define LUX_TO_PPDF_CONST 1/54
#define LUX_READING_PERIOD_S 5
#define SEC_TO_US 1000000
#define PPFD_LED 100
#define TIME_LED_ON 05
#define TIME_LED_OFF 21

void (*func_Pointer)();

void printLocalTime();
static void timer_hourly_callback(void *arg);
static void timer_luxReading_callback(void *arg);
void printBatteryStats();
void chargeOn();
void chargeOff();
void requestHourlyPlan();
void idle();
void setupTimers();
void setupBQ27441(void);
void ledTurnOff();
void ledTurnOn();
void processPlan(String msg);
void takeLuxReading();
double ppfdToDli(double ppfd);
double luxToPpfd(uint16_t lux);
void registerAndCheckDli(double dli_contribution);
void checkAndSwitchLed();

esp_timer_handle_t timer_hourly;
esp_timer_handle_t timer_luxReading;

struct tm timeinfo;
double dli_reached = 5;
double dli_goal = 12.0;
boolean LED_On = false;
int previousHour = 0;

String ans = "init";

const unsigned int BATTERY_CAPACITY = 8000; 

BQ27441 lipo_1;  
unsigned int soc2;
void setup()
{
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  pinMode(CHARGE_RELAY_PIN,OUTPUT);
  digitalWrite(13,HIGH);
  delay(5000);
  initLuxSensor();
  setupBQ27441();
  configureWiFi();
  setupTimers();
  func_Pointer = idle;

}
void setupTimers(){
  // setup Hourly Timer
  esp_timer_create_args_t hourlyTimer_args;
  hourlyTimer_args.callback = &timer_hourly_callback;
  hourlyTimer_args.name = "hourlyTimer";

  //Setup Light Reading Timer
  esp_timer_create_args_t luxReadingTimer_args;
  luxReadingTimer_args.callback = &timer_luxReading_callback;
  luxReadingTimer_args.name = "luxReadingTimer";

  
  ESP_ERROR_CHECK(esp_timer_create(&hourlyTimer_args, &timer_hourly));
  ESP_ERROR_CHECK(esp_timer_create(&luxReadingTimer_args,&timer_luxReading));

   ESP_ERROR_CHECK(esp_timer_start_periodic(timer_luxReading,LUX_READING_PERIOD_S*SEC_TO_US));

  // Ska nog inte startas här?
  getLocalTime(&timeinfo);
  int timeToClosestHour_us = (60 - timeinfo.tm_min)*60*1000000;

   ESP_ERROR_CHECK(esp_timer_start_once(timer_hourly, timeToClosestHour_us ));
}

static void timer_hourly_callback(void *arg)
{   
    func_Pointer = requestHourlyPlan;
    
    ESP_ERROR_CHECK(esp_timer_start_once(timer_hourly, 3600000000 ));
    
}
static void timer_luxReading_callback(void *arg){
  
  func_Pointer = takeLuxReading;
    
}

void setupBQ27441(void)
{
  Wire.begin();
  
  if (!lipo_1.begin(&Wire))
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
  lipo_1.setCapacity(BATTERY_CAPACITY);
  
}

void printBatteryStats()
{
  // Read battery stats from the BQ27441-G1A
  //unsigned int soc = lipo_1.soc();  // Read state-of-charge (%)
  soc2 = lipo_1.soc();  // Read state-of-charge (%)
  int current = lipo_1.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo_1.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo_1.capacity(REMAIN); // Read remaining capacity (mAh)
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

void turnOffCharge(){
  digitalWrite(13,!digitalRead(13));

}
void printLocalTime(){
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
}

void requestHourlyPlan(){
  getLocalTime(&timeinfo);
  String msg = String(dli_reached)+"_";
  //msg += String(lipo_1.soc());
  msg+= "15_16";
  //msg += String(timeinfo.tm_hour);
  javaServerRequest(1,msg);
  delay(100);
  ans = javaServerRequest(2,"ne");
  Serial.println(ans);
  processPlan(ans);
  func_Pointer = idle;
  
}
void processPlan(String msg){

  String charge_str = msg.substring(0,1);
  double DLI_goal_new = msg.substring(2).toDouble();

  if(charge_str.equals("1")){
    chargeOn();
  } else if(charge_str.equals("0")){
    chargeOff();
  } else {
    // ToDo här skulle man kunna ha någon semi-smart lösning så att den inte dör?
    Serial.println("Error in charge message from Server");
  }

  dli_goal = DLI_goal_new;

  


}

void takeLuxReading(){

 uint16_t reading = readLuxVisible();
  Serial.println("reading: "+String(reading));
 registerAndCheckDli( ppfdToDli( luxToPpfd( reading ) ) );

 func_Pointer = checkAndSwitchLed;
  
}

double luxToPpfd(uint16_t lux){

  return lux*LUX_TO_PPDF_CONST;
}
double ppfdToDli(double ppfd){

  return ppfd * 0.000001 * LUX_READING_PERIOD_S;
}
boolean isDliReached(){
  return dli_reached >= dli_goal;
}
void checkAndSwitchLed(){

  getLocalTime(&timeinfo);
  int hourNow = timeinfo.tm_hour;

  // detects new day and resets dli_reached
  if(hourNow < previousHour){
    dli_reached = 0;
  }
  previousHour = hourNow;

  if(hourNow >= TIME_LED_ON && hourNow < TIME_LED_OFF && !isDliReached()){
    ledTurnOn();
  }
  func_Pointer = idle;
}
void registerAndCheckDli(double dli_contribution){

Serial.print("Added dli: ");
Serial.println(dli_contribution,7);
  dli_reached += dli_contribution;

  if(LED_On){

    dli_reached += luxToPpfd(PPFD_LED);

    if( isDliReached() ){

      ledTurnOff();
    }
  }

  
}
void chargeOn(){
  Serial.println("ChargeOn");
  digitalWrite(CHARGE_RELAY_PIN,LOW);
}
void chargeOff(){
  Serial.println("ChargeOFF");
  digitalWrite(CHARGE_RELAY_PIN,HIGH);
}
void ledTurnOff(){
  LED_On = false;
  digitalWrite(LED_PIN,LOW);
}
void ledTurnOn(){
  LED_On = true;
  digitalWrite(LED_PIN,HIGH);
}
void idle(){
Serial.println("idle");
}



void loop() 
{

 
  func_Pointer();
  printLocalTime();
  //Serial.println(ans);

   delay(2000);
}

