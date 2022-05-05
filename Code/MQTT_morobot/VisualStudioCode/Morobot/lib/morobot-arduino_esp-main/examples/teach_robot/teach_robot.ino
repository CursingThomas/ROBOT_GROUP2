/**
 *  \file teach_robot.ino
 *  \brief Move the robot around and store positions using the Dabble-App. The robot can than drive to these positions autonomously. You can also export all positions as movement comments.
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  
 *  Hardware: 		- Arduino Mega (or similar microcontroller)
 *  				- HC-05 bluetooth controller (or similar)
 *  				- morobot
 *  				- Powersupply 9-12V 5A (or more)
 *  Connections:	- Powersupply to Arduino hollow connector
 *  				- First smart servo of robot to Arduino:
 *  					- Red cable to Vin
 *  					- Black cable to GND
 *  					- Yellow cable to pin 16 (TX2)
 *  					- White calbe to pin 17 (RX2)
 *  				- HC-05 to Arduino (Configuration to work with Dabble):
 *  					- 5V to 5V
 *  					- GND to GND
 *  					- RX to pin 14 (TX3)
 *  					- TX to pin 15 (RX3)
 *  Install the Dabble-App on your smartphone or tablet
 */

// **********************************************************************
// ********************* CHANGE THESE TWO LINES *************************
// **********************************************************************
#define MOROBOT_TYPE 	morobot_s_rrr	// morobot_s_rrr, morobot_s_rrp, morobot_2d, morobot_3d, morobot_p
#define SERIAL_PORT		"Serial2"		// "Serial", "Serial1", "Serial2", "Serial3" (not all supported for all microcontroller - see readme)

#include <morobot.h>
#include <dabble_include.h>

#define MAX_NUM_POS 20
#define NUM_SERVOS 3

long motorAngles[NUM_SERVOS][MAX_NUM_POS];
int delayTime = 1500;
int delayDebounce = 500;
int posId = 0;
int idxPlayback = -1;

MOROBOT_TYPE morobot;

void setup() {
	Dabble.begin(DABBLE_PARAM);
	morobot.begin(SERIAL_PORT);
	morobot.moveHome();	
	delay(500);
	morobot.releaseBreaks();
	morobot.setSpeedRPM(2);
	
	Serial.println("Waiting for Dabble to connect to smartphone. If you are already connected, press any app-key.");
	Dabble.waitForAppConnection();
	Serial.println("Dabble connected!");
	
	Serial.println("'Select' to store position.");
	Serial.println("'Start' to start moving.");
	Serial.println("'O' to move to init position.");
	Serial.println("'Left'/'Right' to jump through positions stored.");
	Serial.println("'Up'/'Down' to move last axis.");
	Serial.println("'X' to release breaks.");
	Serial.println("Square-symbol to export positions to the serial monitor.");
}

void loop() {
	Dabble.processInput();

	if (morobot.type == "morobot_s_rrp"){	
		if(GamePad.isPressed(0)) {			// Up
			morobot.moveAngle(2, -20, 30);
		} else if(GamePad.isPressed(1)) {   // Down
			morobot.moveAngle(2, 20, 30);
		}
	}
	
	if(GamePad.isPressed(2)) {	// Left
		if (idxPlayback == -1){
			Serial.println("Store positions before going back");
		} else {
			long angles[3] = {motorAngles[0][idxPlayback], motorAngles[1][idxPlayback], motorAngles[2][idxPlayback]};
			morobot.moveToAngles(angles);
			delay(delayTime);
			idxPlayback--;
			if (idxPlayback == -1) idxPlayback = posId - 1;	// if last position is reached
		}
		delay(delayDebounce);
	} else if(GamePad.isPressed(3)) {   // Right
		if (idxPlayback == -1){
			Serial.println("Store positions before going through them");
		} else {
			long angles[3] = {motorAngles[0][idxPlayback], motorAngles[1][idxPlayback], motorAngles[2][idxPlayback]};
			morobot.moveToAngles(angles);
			delay(delayTime);
			idxPlayback++;
			if (idxPlayback == posId) idxPlayback = 0;	// if last position is reached
		}
		delay(delayDebounce);
	} else if(GamePad.isPressed(7)) {	// O
		morobot.moveHome();
	} else if(GamePad.isPressed(8)) {   // X
		morobot.releaseBreaks();
	} else if(GamePad.isPressed(4)) {	// Start
		for (int i=0; i<posId; i++) {
			long angles[3] = {motorAngles[0][i], motorAngles[1][i], motorAngles[2][i]};
			morobot.moveToAngles(angles);
			delay(delayTime);
		}
	} else if(GamePad.isPressed(5)) {	// Select
		if (posId < MAX_NUM_POS){
			for (byte i=0; i<NUM_SERVOS; i++) motorAngles[i][posId] = morobot.getActAngle(i);
			Serial.print("Stored position ");
			Serial.print(posId);
			Serial.print(": ");
			printAnglesFromIdx(posId);
			idxPlayback = posId;
			posId++;
		} else {
			Serial.println("Maximum number ob positions reached");
		}
		delay(delayDebounce);	
	} else if(GamePad.isPressed(6)) {	//Triangle
		Serial.println(morobot.getCurrent(1));
		Serial.println(morobot.getVoltage(1));
		Serial.println(morobot.getTemp(1));
		Serial.println(morobot.getSpeed(1));
		delay(delayDebounce);
	} else if(GamePad.isPressed(9)) {   // Square
		Serial.println("START COPYING HERE");
		for (int i=0; i<posId; i++) {
			Serial.print("morobot.moveToAngles(");
			Serial.print(motorAngles[0][i]);
			Serial.print(", ");
			Serial.print(motorAngles[1][i]);
			Serial.print(", ");
			Serial.print(motorAngles[2][i]);
			Serial.println(");");
		}
		Serial.println("STOP COPYING HERE");
		Serial.println("Just paste the code into the loop of the base_importantFunctionCalls-example (or make a new file with it).");
	}
}

void printAnglesFromIdx(long idx){
	long ang[NUM_SERVOS];
	for (byte i=0; i<NUM_SERVOS; i++) ang[i] = motorAngles[i][idx];
	morobot.printAngles(ang);
}
