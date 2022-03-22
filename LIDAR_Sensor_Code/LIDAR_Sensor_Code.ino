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


// You have to connect SDA and SCL as normal.
// In addition, one "XSHUT" line must be connected to Arduino for each LIDAR
int sensor1_EN_Pin = 10;  // Connect to XSHUT
int sensor2_EN_Pin = 9;  
// if you're curious, XSHUT stands for NOT_SHUTDOWN, 
//   which is synonomous with ENABLE, shortend to EN.

#include <Wire.h>
#include <VL53L1X.h>
#include "SevSeg.h"


VL53L1X sensor1;
VL53L1X sensor2;
SevSeg sevseg; 
int LEDcounter = 0;

bool s1s = false;
bool s2s = false;

bool s1f = false;
bool s2f = false;

const int ACTIVATION_DISTANCE = 500; //distance at which the lidars will be triggered, in mm

const int MAX_OCCUPANCY = 10;

const int RED = 11;
const int YELLOW = 12;
const int GREEN = 13;

void setup()
{
	pinMode(RED, OUTPUT);
	pinMode(YELLOW, OUTPUT);
	pinMode(GREEN, OUTPUT);

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

	sensor1.read();
	sensor2.read();
	sevseg.setNumber(1);
	sevseg.refreshDisplay();

	Serial.print("range1: ");
	Serial.print(sensor1.ranging_data.range_mm);
	Serial.print(", range2: ");
	Serial.print(sensor2.ranging_data.range_mm);
	
	if (sensor1.ranging_data.range_mm < ACTIVATION_DISTANCE) {
		if (!s1s) {
			s1s = true;
			if (s2f) {
				LEDcounter++;
				s2f = false;
			} else {
				s1f = true;
			}
		}
	} else {
		s1s = false;
	}

	if (sensor2.ranging_data.range_mm < ACTIVATION_DISTANCE) {
		if (!s2s) {
			s2s = true;
			if (s1f) {
				LEDcounter--;
				s1f = false;
			} else {
				s2f = true;
			}
		}
	} else {
		s2s = false;
	}

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

}
