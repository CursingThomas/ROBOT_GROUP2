/**
 *  \file driveAround.ino
 *  \brief Control the with the dabble app and drive around giving x-y-z-coordinates or angle values
 *  @author	Johannes Rauer FHTW
 *  @date	2020/02/23
 *  
 *  Hardware: 		- Arduino Mega (or similar microcontroller)
 *  				- HC-05 bluetooth controller (or similar)
 *  				- Calibrated morobot
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
#define MOROBOT_TYPE 	morobot_p		// morobot_s_rrr, morobot_s_rrp, morobot_2d, morobot_3d, morobot_p
#define SERIAL_PORT		"Serial2"		// "Serial", "Serial1", "Serial2", "Serial3" (not all supported for all microcontroller - see readme)

#include <morobot.h>
#include <dabble_include.h>

// Create morobot object and declare variables
MOROBOT_TYPE morobot;
String drive_type = "xyz";				// xyz, angular
float step = 2.0;
int delayDebounce = 250;
float actPos[3];
float actPosTemp[3];

void setup() {
	Dabble.begin(DABBLE_PARAM);		// Start connection to Dabble
	morobot.begin(SERIAL_PORT);
	morobot.waitAfterEachMove = false;	// Make sure the robot moves without waiting
	morobot.setSpeedRPM(1);
	//morobot.setTCPoffset(20, 50, 50); // If the robot has an endeffector set its position here (e.g. Pen-Holder)
	morobot.moveHome();				// Move the robot into initial position
	delay(200);
	initVars();
	
	if (morobot.type == "morobot_s_rrp") step = 1;
	
	Serial.println("Waiting for Dabble to connect to smartphone. If you are already connected, press any app-key.");
	Dabble.waitForAppConnection();
	Serial.println("Dabble connected!");
	Serial.println("Use the gamepad (left/right/up/down/Triangle/X) to drive the robot around.");
}

void loop() {
	// Get the input from the app
	Dabble.processInput();

	if(GamePad.isPressed(7)) {	// O
		morobot.waitAfterEachMove = true;
		morobot.moveHome();
		morobot.waitAfterEachMove = false;
		initVars();
	} else if(GamePad.isPressed(9)) {   // Square
		long angles[3] = {morobot.getActAngle(0), morobot.getActAngle(1), morobot.getActAngle(2)};
		morobot.printTCPpose();
		morobot.printAngles(angles);
		delay(delayDebounce);
		initVars();
	} else if(GamePad.isPressed(5)) {	// Select
		if (drive_type == "xyz") {
			drive_type = "angular";
			step = step*2;
			Serial.println(F("Changed Drive-Type to ANGULAR"));
		} else {
			drive_type = "xyz";
			step = step/2;
			Serial.println(F("Changed Drive-Type to XYZ"));
			initVars();
		}
		delay(delayDebounce);
	}

	if (drive_type == "xyz"){
		if(GamePad.isPressed(2)) {			// Left
			actPosTemp[1] = actPos[1] - step;
		} else if(GamePad.isPressed(3)) {	// Right
			actPosTemp[1] = actPos[1] + step;
		} else if(GamePad.isPressed(0)) {	// Up
			actPosTemp[0] = actPos[0] - step;
		} else if(GamePad.isPressed(1)) {	// Down
			actPosTemp[0] = actPos[0] + step;
		} else if(GamePad.isPressed(6)) {	// Triangle
			if (morobot.type == "morobot_s_rrp") actPosTemp[2] = actPos[2] + 0.05;
			else actPosTemp[2] = actPos[2] + step;
		} else if(GamePad.isPressed(8)) {   // X
			if (morobot.type == "morobot_s_rrp") actPosTemp[2] = actPos[2] - 0.05;
			else actPosTemp[2] = actPos[2] - step;
		}
	
		// If a movement comment has been given, move the robot
		if (actPosTemp[0] != actPos[0] || actPosTemp[1] != actPos[1] || actPosTemp[2] != actPos[2]){
			if (morobot.moveToPose(actPos[0], actPos[1], actPos[2]) == false){
				delay(delayDebounce);
				initVars();
			} else {
				for (uint8_t i=0; i<3; i++) actPos[i] = actPosTemp[i];
			}
		}
	}
	
	if (drive_type == "angular"){
		if(GamePad.isPressed(2)) {			// Left
			morobot.moveAngle(0, step);
		} else if(GamePad.isPressed(3)) {	// Right
			morobot.moveAngle(0, -step);
		} else if(GamePad.isPressed(0)) {	// Up
			morobot.moveAngle(1, step);
		} else if(GamePad.isPressed(1)) {	// Down
			morobot.moveAngle(1, -step);
		} else if(GamePad.isPressed(6)) {	// Triangle
			morobot.moveAngle(2, step);
		} else if(GamePad.isPressed(8)) {   // X
			morobot.moveAngle(2, -step);
		}
		delay(20);
	}
}

void initVars(){
	actPos[0] = morobot.getActPosition('x');
	actPos[1] = morobot.getActPosition('y');
	actPos[2] = morobot.getActPosition('z');
	for (uint8_t i=0; i<3; i++) actPosTemp[i] = actPos[i];
}
