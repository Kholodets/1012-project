#include <Wire.h>
#include <VL53L1X.h>

VL53L1X sensor;
int buzzer = 4;
int distance = 0;

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
}

void loop()
{
  distance = sensor.read();
  Serial.print(distance);

  if (distance < 100){ //less than 5 cm
    tone(buzzer, 200);
  } else {
    noTone(buzzer);  
  }
  delay(20);
  Serial.println();
}
