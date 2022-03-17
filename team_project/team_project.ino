#include <Wire.h>
#include <VL53L1X.h>
#include "SevSeg.h"

SevSeg sevseg; 
VL53L1X sensor;
int buzzer = 10;
int distance = 0;
int LEDcounter = 0;

bool openState = false;

void setup()
{
  while (!Serial) {}
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(2000);
  pinMode(buzzer,OUTPUT);

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }

  

  // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but
  // the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more
  // information on range and timing limits.
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);

  // Start continuous readings at a rate of one measurement every 50 ms (the
  // inter-measurement period). This period should be at least as long as the
  // timing budget.
  sensor.startContinuous(50);
  Serial.println("Sensor initialized");

   byte numDigits = 1;
    byte digitPins[] = {};
    byte segmentPins[] = {6, 5, 2, 3, 4, 7, 8, 9};
    bool resistorsOnSegments = true;

    byte hardwareConfig = COMMON_ANODE; 
    sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
    sevseg.setBrightness(90);

    Serial.println("LED Initialized");
}

void loop()
{
  distance = sensor.read();
  Serial.print(distance);

  if (distance < 100){ //less than 5 cm
    tone(buzzer, 200);
    //LEDcounter++;
    if(!openState) {
      LEDcounter++;
      openState = true;
    }

  } else {
    noTone(buzzer);  
    openState = false;
  }
  
  sevseg.setNumber(LEDcounter % 10);
  sevseg.refreshDisplay();
  delay(20);
  Serial.println();
}
