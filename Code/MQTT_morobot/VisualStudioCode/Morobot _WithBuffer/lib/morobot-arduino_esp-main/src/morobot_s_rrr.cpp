/**
 *  \class 	morobot_s_rrr
 *  \brief 	morobot child class for morobot-s (rrr) for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_s_rrr.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
 *  		morobot_s_rrr() : morobotClass(PUT_NUM_SERVOS_HERE){}
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
			bool checkIfAnglesValid(float phi1, float phi2, float phi3);
		protected:
			virtual bool calculateAngles(float x, float y, float rotZ);
			virtual void updateCurrentXYZ();
 */
 
#include "morobot_s_rrr.h"

void morobot_s_rrr::setTCPoffset(float xOffset, float yOffset, float zOffset){
	_tcpOffset[0] = xOffset;
	_tcpOffset[1] = yOffset;
	_tcpOffset[2] = zOffset;
	
	// At the moment, only x/z-offsets are valid!
	if (yOffset != 0) Serial.println(F(" ********************************** WARNING: Y-OFFSETS OF TCP ARE NOT SUPPORTED! **********************************"));
	_tcpOffset[1] = 0;
	
	// Calculate new length and angle of last axis (since eef is connected to it statically)	
	d = xOffset;
	dSQ = pow(d,2);
	bSQ = pow(b,2);
	cSQ = pow(c,2);
	
	_tcpPoseIsValid = false;
}

bool morobot_s_rrr::checkIfAngleValid(uint8_t servoId, float angle){
	// The values are NAN if the inverse kinematics does not provide a solution
	if(!checkForNANerror(servoId, angle)) return false;
	
	// Moving the motors out of the joint limits may harm the robot's mechanics
	if(angle < _jointLimits[servoId][0] || angle > _jointLimits[servoId][1]){
		printInvalidAngleError(servoId, angle);
		return false;
	}
	
	return true;
}

bool morobot_s_rrr::checkIfAnglesValid(float phi1, float phi2, float phi3){
	float angles[3] = {phi1, phi2, phi3};
	
	for (uint8_t i = 0; i < _numSmartServos; i++) if(checkIfAngleValid(i, angles[i]) == false) return false;
	return true;
}

String morobot_s_rrr::getType(){
	return type;
}

/* PROTECTED FUNCTIONS */
bool morobot_s_rrr::calculateAngles(float x, float y, float rotZ){
	rotZ = convertToRad(rotZ);			// Transform rotation into radians
	x = x-a;							// Base is in x-orientation --> Just subtract base-length from x-coordinate
	
	// Calculate position for center of rotation of last axis
	float x_w = x - d*cos(rotZ);
	float y_w = y - d*sin(rotZ);
	float x_wSQ = pow(x_w, 2);
	float y_wSQ = pow(y_w, 2);
	
	float phi2 = acos((x_wSQ + y_wSQ - bSQ - cSQ)/(2 * b * c));
	float gamma = acos((x_wSQ + y_wSQ + bSQ - cSQ)/(2 * b * sqrt(x_wSQ + y_wSQ)));
	float alpha = atan2(y_w, x_w);
	float phi1 = - (alpha - gamma);
	float phi3 = - (rotZ - (phi2 - phi1));
	
	phi1 = convertToDeg(phi1);
	phi2 = convertToDeg(phi2);
	phi3 = convertToDeg(phi3);
	
	// Check if angles are valid
	if (!checkIfAnglesValid(phi1, phi2, phi3)){
		// Try out redundant configuration
		Serial.println(F("Switching to other configuration"));
		phi1 = - (-phi1 + 2*gamma);
		phi3 = - (-phi3 + 2*(phi2-gamma));
		phi2 = - phi2;
		
		phi1 = convertToDeg(phi1);
		phi2 = convertToDeg(phi2);
		phi3 = convertToDeg(phi3);
		if (!checkIfAnglesValid(phi1, phi2, phi3)) return false;
	}
	_goalAngles[0] = phi1;		// Since motor 0 is installed in other direction
	_goalAngles[1] = phi2;
	_goalAngles[2] = phi3;
	
	return true;
}

void morobot_s_rrr::updateTCPpose(bool output){
	if (_tcpPoseIsValid) return;
	
	waitUntilIsReady();
	
	// Get anlges of all motors
	float actAngles[_numSmartServos];
	for (uint8_t i=0; i<_numSmartServos; i++) actAngles[i] = convertToRad(getActAngle(i));
	
	// Change orientation or angle because of motor mounting orientation
	actAngles[0] = -actAngles[0];
	
	// Calculate lengths at each joint and sum up
	float xnb = b*cos(actAngles[0]);
    float ynb = b*sin(actAngles[0]);
    float xnc = c*cos(actAngles[0] + actAngles[1]);
    float ync = c*sin(actAngles[0] + actAngles[1]);
    float xnd = d*cos(actAngles[0] + actAngles[1] - actAngles[2]);
    float ynd = d*sin(actAngles[0] + actAngles[1] - actAngles[2]);
    
	_actPos[0] = a + xnb + xnc + xnd;
    _actPos[1] = ynb + ync + ynd;
	_actPos[2] = _tcpOffset[2];
	
	// Calculate orientation
	_actOri[0] = 0;
	_actOri[1] = 0;
	_actOri[2] = convertToDeg(actAngles[0] + actAngles[1] - actAngles[2]);
	
	if (output == true)	printTCPpose();
	
	_tcpPoseIsValid = true;
}