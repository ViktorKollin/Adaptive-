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
esp_timer_handle_t timer_hourly;
esp_timer_handle_t timer_luxReading;

struct tm timeinfo;
double dli_reached = 0;
double dli_goal = 15.0;
boolean LED_On = false;

String ans = "init";

const unsigned int BATTERY_CAPACITY = 8000; 

BQ27441 lipo;  
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

   //ESP_ERROR_CHECK(esp_timer_start_periodic(timer_luxReading,LUX_READING_PERIOD_S*SEC_TO_US));

  // Ska nog inte startas här?
   ESP_ERROR_CHECK(esp_timer_start_once(timer_hourly, 3000000 ));
}

static void timer_hourly_callback(void *arg)
{   
    func_Pointer = requestHourlyPlan;
    
    ESP_ERROR_CHECK(esp_timer_start_once(timer_hourly, 5000000 ));
    
}
static void timer_luxReading_callback(void *arg){
  /* TODO läs värde
          lägg till total
          kolla om dagens uppnåt (isf stäng av lampa och sätt av)
          
  getLocalTime(&timeinfo);
  Serial.print(&timeinfo,"%H:%M:%S ");   
  */ 
  uint16_t reading = readLuxVisible();
  double ppfd = readLuxVisible()*LUX_TO_PPDF_CONST;
  if(LED_On){
    ppfd += PPFD_LED;
  }
  dli_reached += ( ppfd * 0.000001 * LUX_READING_PERIOD_S );
  if (LED_On && dli_reached >= dli_goal){
    ledTurnOff();
  }
  
  Serial.println(reading);     
}

void setupBQ27441(void)
{
  Wire.begin();
  
  if (!lipo.begin(&Wire))
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
  lipo.setCapacity(BATTERY_CAPACITY);
  
}

void printBatteryStats()
{
  // Read battery stats from the BQ27441-G1A
  //unsigned int soc = lipo_1.soc();  // Read state-of-charge (%)
  soc2 = lipo.soc();  // Read state-of-charge (%)
  int current = lipo.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo.capacity(REMAIN); // Read remaining capacity (mAh)
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
  String msg = String(dli_reached)+"_";
  msg += String(lipo.soc());
  javaServerRequest(0,msg);
  delay(1000);
  ans = javaServerRequest(1,"ne");
  Serial.println(ans);
  func_Pointer = idle;
  
  //Serial.println(ans);
  /*if(ans.equals("Charge")){
   chargeOn();
  }else if(ans.equals("No")){
    chargeOff();
  }*/
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
  //Serial.println(ans);

   delay(1000);
}

