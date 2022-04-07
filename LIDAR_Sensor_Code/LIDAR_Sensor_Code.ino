/* Based on the code Prof. Orser sent us. 
 *  Group Th5
 */
#include <Wire.h>
#include <VL53L1X.h>
#include "SevSeg.h"

int sensor1_EN_Pin = A2;
int sensor2_EN_Pin = A3;

const int buttonRED = A1;
const int buttonBLUE = A0;

const int RED = 11;
const int YELLOW = 12;
const int GREEN = 13;
int LED10 = 9;
int LED20 = 10;

bool buttonREDstate = false;
bool buttonBLUEstate = false;
bool bothButtons = false;

VL53L1X sensor1;
VL53L1X sensor2;
SevSeg sevseg; 

int LEDcounter = 0;

bool sensor1state = false;
bool sensor2state = false;
bool sensor1first = false;
bool sensor2first = false;

//default distance at which the lidars will be triggered, in mm
const int DEFAULT_ACTIVATION_DISTANCE = 500;

const int MAX_OCCUPANCY = 35;
const float PROPORTION = 0.5; //proportion of the read distance to be activation distance

int s1ActivationDistance;
int s2ActivationDistance;

void calibrate(){
	//reads new distance
	sensor1.read();
	sensor2.read();

	//set the new activation distance as a proportion of the read distance
	s1ActivationDistance = PROPORTION*sensor1.ranging_data.range_mm;
	s2ActivationDistance = PROPORTION*sensor2.ranging_data.range_mm;
}

void setup()
{
	pinMode(RED, OUTPUT);
	pinMode(YELLOW, OUTPUT);
	pinMode(GREEN, OUTPUT);

	pinMode(LED10, OUTPUT);
	pinMode(LED20, OUTPUT);

	pinMode(buttonRED, INPUT);
	pinMode(buttonBLUE, INPUT);

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

	sensor1.setDistanceMode(VL53L1X::Long);
	sensor1.setMeasurementTimingBudget(50000);

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

	//set default activation for sensor 1 and 2
	s1ActivationDistance = DEFAULT_ACTIVATION_DISTANCE;
	s2ActivationDistance = DEFAULT_ACTIVATION_DISTANCE;

	//setup for 7 segment
	byte numDigits = 1;
	byte digitPins[] = {};
	byte segmentPins[] = {6,7,8,5,4,2,3};
	bool resistorsOnSegments = true;
	byte hardwareConfig = COMMON_ANODE; 

	sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments);
	sevseg.setBrightness(90);
	Serial.println("7-segment Initialized");

}

void loop()
{
	//red button decrements
	if (digitalRead(buttonRED) == HIGH) {
		if (buttonREDstate) {
			if (!bothButtons) {
				//Serial.println("Button RED released");
				LEDcounter--;
			}
			buttonREDstate = false;
		}
	} else {
		buttonREDstate = true;
		//Serial.println("button 1 pressed");
	}

	//blue button increments
	if (digitalRead(buttonBLUE) == HIGH) {
		if (buttonBLUEstate) {
			if (!bothButtons) {
				//Serial.println("Button BLUE released");
				LEDcounter++;
			}
			buttonBLUEstate = false;
		}
	} else {
		buttonBLUEstate = true;
	}

	//both buttons calibrates
	if (buttonREDstate && buttonBLUEstate) {
		if (!bothButtons){
			calibrate();
			Serial.println("Sensor activation distance recalibrated");
			Serial.print("s1: ");
			Serial.print(s1ActivationDistance);
			Serial.print(", s2: ");
			Serial.println(s2ActivationDistance);
		}
		bothButtons = true;
	}

	if (!buttonREDstate && !buttonBLUEstate) {
		bothButtons = false;
	}

	//read distances from both sensors
	sensor1.read();
	sensor2.read();

	if (sensor1.ranging_data.range_mm < s1ActivationDistance) {
		//if the sensor is NEWLY activated, detirmine if it is 
		//the first or second sensor to be tripped
		if (!sensor1state) {
			sensor1state = true;
			//if it is the second sensor to be activated, reset 
			//the system and increment the counter
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
	if (sensor2.ranging_data.range_mm < s2ActivationDistance) {
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

	//display the current occupancy with LED and 7 segment displays
	//blue LED corresponds to tens place with binary, 7 segment shows 1s place with arabic digits
	if (LEDcounter < 10){ //also accounts for negative overflow
		sevseg.setNumber(LEDcounter);
		digitalWrite(LED10, LOW);
		digitalWrite(LED20, LOW);
	} else if (LEDcounter < 20){
		sevseg.setNumber(LEDcounter % 10);
		digitalWrite(LED10, HIGH);
		digitalWrite(LED20, LOW);
	} else if (LEDcounter < 30) {
		sevseg.setNumber(LEDcounter % 10);
		digitalWrite(LED10, LOW);
		digitalWrite(LED20, HIGH);
	} else if (LEDcounter < 40) {
		sevseg.setNumber(LEDcounter % 10);
		digitalWrite(LED10, HIGH);
		digitalWrite(LED20, HIGH); 
	} else { //overflow
		digitalWrite(LED10, HIGH);
		digitalWrite(LED20, HIGH);
		sevseg.setNumber(LEDcounter);
	}
	sevseg.refreshDisplay();

	//Serial.println(LEDcounter);
	/*Serial.print("Current count: ");
	  Serial.println(LEDcounter);
	  Serial.print("range1: ");
	  Serial.print(sensor1.ranging_data.range_mm);
	  Serial.print(", range2: ");
	  Serial.println(sensor2.ranging_data.range_mm);*/

}
