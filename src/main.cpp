
#include <SparkFunBQ27441.h>
#include <Wire.h>
#define SDA_2 33
#define SCL_2 32

// Set BATTERY_CAPACITY to the design capacity of your battery.
const unsigned int BATTERY_CAPACITY = 80000; // e.g. 850mAh battery

BQ27441 lipo_1; 
BQ27441 lipo_2; 


void setupBQ27441(void)
{
  // Use lipo.begin() to initialize the BQ27441-G1A and confirm that it's
  // connected and communicating.
 
  Wire1.begin(SDA_2,SCL_2);
  
  if (!lipo_1.begin(&Wire1))
  {
    Serial.println("Error: Unable to communicate with BQ27441.");
    Serial.println("  Check wiring and try again.");
    Serial.println("  (Battery must be plugged into Battery Babysitter!)");
    while (1) ;
  }
  Serial.println("Connected to first BQ27441!");
  
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
  lipo_1.setCapacity(BATTERY_CAPACITY);
  lipo_2.setCapacity(BATTERY_CAPACITY);
}

void printBatteryStats()
{
  // Read battery stats from the BQ27441-G1A
  unsigned int soc = lipo_1.soc();  // Read state-of-charge (%)
  unsigned int soc2 = lipo_2.soc();  // Read state-of-charge (%)
  /*
  unsigned int volts = lipo.voltage(); // Read battery voltage (mV)
  int current = lipo.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo.capacity(REMAIN); // Read remaining capacity (mAh)
  int power = lipo.power(); // Read average power draw (mW)
  int health = lipo.soh(); // Read state-of-health (%)
  */

  // Now print out those values:
  String toPrint = "First Battery " + String(soc) + "% | ";
   toPrint += "Second battery " + String(soc2) + " % | ";
  /*
  toPrint += String(volts) + " mV | ";
  toPrint += String(current) + " mA | ";
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
  setupBQ27441();
}

void loop() 
{
  printBatteryStats();
  delay(1000);
}