/**
 *  \class 	morobot_p
 *  \brief 	morobot child class for morobot-p for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_p.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
 *  		morobot_p() : morobotClass(3){};
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
			void moveHome();
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			virtual void updateCurrentXYZ();
 */
 
#include "morobot_p.h"

void morobot_p::setTCPoffset(float xOffset, float yOffset, float zOffset){
	// Instead of using the tcp-offset in x-direction, make the last link longer (but store the offset just to be sure)
	_tcpOffset[0] = xOffset;
	a3 = a3 + xOffset;
	_tcpOffset[1] = yOffset;
	_tcpOffset[2] = zOffset;
	_tcpPoseIsValid = false;
	
	// At the moment, only x/z-offsets are valid!
	if (yOffset != 0) Serial.println(F(" ********************************** WARNING: Y-OFFSETS OF TCP ARE NOT SUPPORTED! **********************************"));
	_tcpOffset[1] = 0;
}

bool morobot_p::checkIfAngleValid(uint8_t servoId, float angle){
	// The values are NAN if the inverse kinematics does not provide a solution
	if(!checkForNANerror(servoId, angle)) return false;
	
	// Moving the motors out of the joint limits may harm the robot's mechanics
	if(angle < _jointLimits[servoId][0] || angle > _jointLimits[servoId][1]){
		printInvalidAngleError(servoId, angle);
		return false;
	}
	
	// Check difference between servo1 and servo2 (in case only one motor moves at a time)	
	if (servoId == 1 || servoId == 2) {
		long servo1Angle = angle;
		long servo2Angle = angle;
		if (servoId == 1) servo2Angle = getActAngle(2);
		else if (servoId == 2) servo1Angle = getActAngle(1);
		return checkIfAngleDiffValid(servo1Angle, servo2Angle);
	}
	
	return true;
}

bool morobot_p::checkIfAngleDiffValid(float servo1Angle, float servo2Angle){
	if (90 - servo1Angle - servo2Angle < 20){
		Serial.println(F("Difference between motor2 and motor3 too small - could harm mechanics"));
		_tcpPoseIsValid = false;
		return false;
	}
	
	if (90 - servo1Angle - servo2Angle > 135){
		Serial.println(F("Difference between motor2 and motor3 too big - could harm mechanics"));
		_tcpPoseIsValid = false;
		return false;		
	}
	
	return true;
}

// Alternate home position which is not in minus-z-direction
void morobot_p::moveHome(){
	moveToAngle(0, 0);
	moveToAngle(1, 75);
	moveToAngle(2, -25);
	waitUntilIsReady();
}

String morobot_p::getType(){
	return type;
}

/* PROTECTED FUNCTIONS */
bool morobot_p::calculateAngles(float x, float y, float z){
	// Subtract offset
	x = x - x_def_offset;	// Don't use x-offset because this is already in link a3 included;
	y = y - _tcpOffset[1];
	z = z - _tcpOffset[2] + z_def_offset;
	
	// Helper calculations
	float a1_sq = pow(a1, 2);
	float a2_sq = pow(a2, 2);
	float s_sq = pow(sqrt(pow(x, 2) + pow(y, 2)) - a3, 2) + pow(z - d1, 2);
	float s = sqrt(s_sq);
	
	// Calculate angles
	float theta1 = atan2(y, x);
	float theta2 = acos((a1_sq + s_sq - a2_sq) / (2 * a1 * s)) + asin((z - d1) / s);
	float theta3 = acos((a1_sq - s_sq + a2_sq) / (2 * a1 * a2));
	
	// Change to degrees
	theta1 = convertToDeg(theta1);
	theta3 = -1 * convertToDeg(theta3 + theta2 - M_PI/2);
	theta2 = convertToDeg(theta2);
	
	//Check if angles are valid
	if(checkIfAngleValid(0, theta1) == false) return false;
	if(checkIfAngleValid(1, theta2) == false) return false;
	if(checkIfAngleValid(2, theta3) == false) return false;
	
	// Store values
	_goalAngles[0] = theta1;
	_goalAngles[1] = theta2;
	_goalAngles[2] = theta3;
	
	return true;
}

void morobot_p::updateTCPpose(bool output){
	if (_tcpPoseIsValid) return;
	
	waitUntilIsReady();
	
	// Get motor angles
	float theta1 = getActAngle(0);
	float theta2 = getActAngle(1);
	float theta3 = -getActAngle(2);
	
	// Recalculate angles and convert to radians
    theta3 = theta3 - 90 - theta2;
 	theta1 = convertToRad(theta1);
	theta2 = convertToRad(theta2);
	theta3 = convertToRad(theta3);
	
	float x = a1 * cos(theta1) * cos(theta2) + a2 * cos(theta1) * cos(theta2 + theta3) + a3 * cos(theta1);
	float y = a1 * sin(theta1) * cos(theta2) + a2 * sin(theta1) * cos(theta2 + theta3) + a3 * sin(theta1);
    float z = d1 + a1 * sin(theta2) + a2 * sin(theta2 + theta3);
    
	// Store position
	_actPos[0] = x + x_def_offset;		// Don't use x-offset because this is already in link a3 included;
	_actPos[1] = y + _tcpOffset[1];
	_actPos[2] = z + _tcpOffset[2] - z_def_offset;
	
	// Calculate orientation
	_actOri[0] = 0;
	_actOri[1] = 0;
	_actOri[2] = convertToDeg(theta1);
	
	if (output == true)	printTCPpose();
	
	_tcpPoseIsValid = true;
}