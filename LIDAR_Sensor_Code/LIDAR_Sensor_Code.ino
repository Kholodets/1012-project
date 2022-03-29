/* Based on the code Prof. Orser sent us. 
 *  Group Th5
 */
/*
   This example takes range measurements with the VL53L1X and displays additional 
   details (status and signal/ambient rates) for each measurement, which can help
   you determine whether the sensor1 is operating normally and the reported range is
   valid. The range is in units of mm, and the rates are in units of MCPS (mega 
   counts per second).
 */

int sensor1_EN_Pin = A2;
int sensor2_EN_Pin = A3;

int buttonRED = A1;
int buttonGREEN = A0;

int buttonREDstate;
int buttonGREENstate;

bool buttonREDopen = false;
bool buttonGREENopen = false;

#include <Wire.h>
#include <VL53L1X.h>
#include "SevSeg.h"


VL53L1X sensor1;
VL53L1X sensor2;
SevSeg sevseg; 
int LEDcounter = 0;

bool sensor1state = false;
bool sensor2state = false;

const int ACTIVATION_DISTANCE = 500; //distance at which the lidars will be triggered, in mm

const int MAX_OCCUPANCY = 5;

const int RED = 11;
const int YELLOW = 12;
const int GREEN = 13;
int LED10 = 10;
int LED20 = 9;

void setup()
{
	pinMode(RED, OUTPUT);
	pinMode(YELLOW, OUTPUT);
	pinMode(GREEN, OUTPUT);

  pinMode(LED10, OUTPUT);
  pinMode(LED20, OUTPUT);

  pinMode(buttonRED, INPUT);
  pinMode(buttonGREEN, INPUT);

	pinMode(sensor1_EN_Pin,OUTPUT);
	pinMode(sensor2_EN_Pin,OUTPUT);
	digitalWrite(sensor1_EN_Pin,HIGH); // Enable sensor1
	digitalWrite(sensor2_EN_Pin,LOW); // Disable sensor2


	Serial.begin(115200);
	Wire.begin();
	Wire.setClock(400000); // use 400 kHz I2C

	// First we setup sensor1 as normal
	sensor1.setTimeout(500);

	if (!sensor1.init()) {
		Serial.println("Failed to detect and initialize sensor1!");
		while (1);
	}

	// Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
	// You can change these settings to adjust the performance of the sensor1, but
	// the minimum timing budget is 20 ms for short distance mode and 33 ms for
	// medium and long distance modes. See the VL53L1X datasheet for more
	// information on range and timing limits.
	sensor1.setDistanceMode(VL53L1X::Long);
	sensor1.setMeasurementTimingBudget(50000);

	// Start continuous readings at a rate of one measurement every 50 ms (the
	// inter-measurement period). This period should be at least as long as the
	// timing budget.
	sensor1.startContinuous(50);


	// Next check original sensor1 Address (debugging purposes only.)
	Serial.print("Sensor Detected at:");
	Serial.println(sensor1.getAddress());

	// Now, change Sensor1 to a new address
	sensor1.setAddress(40);
	Serial.print("Sensor Address Changed to:");  // verify it got set correctly.
	Serial.println(sensor1.getAddress());

	// Now that Addresss 41 is free again, enable sensor2
	digitalWrite(sensor2_EN_Pin,HIGH); // Enable sensor2

	// Sensor2 is still at the default address of 41
	// and is setup just like the original example
	sensor2.setTimeout(500);

	if (!sensor2.init()) {
		Serial.println("Failed to detect and initialize sensor2!");
		while (1);
	}

	sensor2.setDistanceMode(VL53L1X::Long);
	sensor2.setMeasurementTimingBudget(50000);
	sensor2.startContinuous(50);

	//setup for 7 segment
	byte numDigits = 1;
	byte digitPins[] = {};
	byte segmentPins[] = {6,7,8,5,4,2,3};
	bool resistorsOnSegments = true;

	byte hardwareConfig = COMMON_ANODE; 
	sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
	sevseg.setBrightness(90);

	Serial.println("LED Initialized");

}

void loop()
{
  buttonREDstate = digitalRead(buttonRED);
  buttonGREENstate = digitalRead(buttonGREEN);

  //red button decrements sensor
  if ((buttonREDstate == HIGH) && (buttonGREENstate == LOW)){
    if (!buttonREDopen){ //sees if button is still being held
      LEDcounter--;
      buttonREDopen = true;
    }
    
  } else if (buttonREDstate == LOW){
    buttonREDopen = false;
  }

  //green button increments sensor
  if ((buttonGREENstate == HIGH) && (buttonREDstate == LOW)){
    if (!buttonGREENstate){ //sees if button is still being held
      LEDcounter++;
      buttonGREENopen = true;
    }
  } else if (buttonGREENstate == LOW){
    buttonGREENopen = false;
  }

    
	//read distances from both sensors
	sensor1.read();
	sensor2.read();

	if (sensor1.ranging_data.range_mm < ACTIVATION_DISTANCE) {
		//if the sensor is NEWLY activated, detirmine if it is the first or second sensor to be tripped
		if (!sensor1state) {
			sensor1state = true;
			//if it is the second sensor to be activated, reset the system and increment the counter
			if (sensor2first) {
				LEDcounter++;
				sensor2first = false;
			//if its the first, prime the second sensor to be ready
			} else {
				sensor1first = true;
			}
		}
	} else {
		sensor1state = false;
	}

	//repeat for sensor two, will decrement the countrer instead of incrementing it
	if (sensor2.ranging_data.range_mm < ACTIVATION_DISTANCE) {
		if (!sensor2state) {
			sensor2state = true;
			if (sensor1first) {
				LEDcounter--;
				sensor1first = false;
			} else {
				sensor2first = true;
			}
		}
	} else {
		sensor2state = false;
	}

	//turn on the LED corresponding to the current occupancy
	if (LEDcounter >= MAX_OCCUPANCY) {
		digitalWrite(RED, HIGH);
		digitalWrite(YELLOW, LOW);
		digitalWrite(GREEN, LOW);
	} else if (((float) LEDcounter) / MAX_OCCUPANCY >= 0.8) {
		digitalWrite(RED, LOW);
		digitalWrite(YELLOW, HIGH);
		digitalWrite(GREEN, LOW);
	} else {
		digitalWrite(RED, LOW);
		digitalWrite(YELLOW, LOW);
		digitalWrite(GREEN, HIGH);
	}
	sevseg.setNumber(LEDcounter);
	sevseg.refreshDisplay();

	Serial.print("Current count: ");
	Serial.println(LEDcounter);
	Serial.print("range1: ");
	Serial.print(sensor1.ranging_data.range_mm);
	Serial.print(", range2: ");
	Serial.println(sensor2.ranging_data.range_mm);



}
