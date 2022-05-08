/**
 *  \class 	morobot_s_rrp
 *  \brief 	morobot child class for morobot-s (rrp) for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_s_rrp.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
 *  		morobot_s_rrp() : morobotClass(3){};
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
			bool checkIfAnglesValid(float phi1, float phi2, float phi3);
			void moveZAxisIn(uint8_t maxMotorCurrent);
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			virtual void updateTCPpose();
 */
 
#include "morobot_s_rrp.h"

void morobot_s_rrp::setTCPoffset(float xOffset, float yOffset, float zOffset){
	_tcpOffset[0] = xOffset;
	_tcpOffset[1] = yOffset;
	_tcpOffset[2] = zOffset;
	
	// Calculate new length and angle of last axis (since eef is connected to it statically)
    c_new = sqrt(pow(_tcpOffset[1],2) + pow(c+_tcpOffset[0],2));
    beta_new = asin(_tcpOffset[1]/c_new);
	
	/*// At the moment, only x/z-offsets are valid!
	if (yOffset != 0) {
		Serial.println(F(" ********************************** WARNING: Y-OFFSETS OF TCP ARE NOT SUPPORTED! **********************************"));
		_tcpOffset[1] = 0;
		c_new = c+_tcpOffset[0];
		beta_new = 0;
	}*/
	
	// Precalculate squares of lengths for faster processing
	c_newSQ = pow(c_new,2);
	bSQ = pow(b,2);
	
	_tcpPoseIsValid = false;
}

bool morobot_s_rrp::checkIfAngleValid(uint8_t servoId, float angle){
	// The values are NAN if the inverse kinematics does not provide a solution
	if(!checkForNANerror(servoId, angle)) return false;
	
	// Moving the motors out of the joint limits may harm the robot's mechanics
	if(angle < _jointLimits[servoId][0] || angle > _jointLimits[servoId][1]){
		printInvalidAngleError(servoId, angle);
		return false;
	}
	
	return true;
}

bool morobot_s_rrp::checkIfAnglesValid(float phi1, float phi2, float phi3){
	float angles[3] = {phi1, phi2, phi3};
	
	for (uint8_t i = 0; i < _numSmartServos; i++) if(checkIfAngleValid(i, angles[i]) == false) return false;
	return true;
}

void morobot_s_rrp::moveZAxisIn(uint8_t maxMotorCurrent){
	autoCalibrateLinearAxis(2, maxMotorCurrent);
	waitUntilIsReady();
	_tcpPoseIsValid = false;
}

String morobot_s_rrp::getType(){
	return type;
}

/* PROTECTED FUNCTIONS */
bool morobot_s_rrp::calculateAngles(float x, float y, float z){
	float xSQ = pow(x-a,2);	// Base is in x-orientation --> Just subtract base-length from x-coordinate
	float ySQ = pow(y,2);
	
	// Calculate angle for 2nd axis
	float phi2n = - acos((xSQ + ySQ - bSQ - c_newSQ) / (2*b*c_new));		// Some terms are negative since motor1+2 are mounted in other direction
	float phi2 = convertToDeg(phi2n - beta_new);
	
	// Calculate angle for 1st axis
	float gamma = atan2(y, x-a);
	float alpha = acos((xSQ + ySQ + bSQ - c_newSQ) / (2*b*sqrt(xSQ + ySQ)));
	float phi1 = - convertToDeg(gamma + alpha);

	// Recalculate angles if phi1 is out of range
	if (phi1 < _jointLimits[0][0] || phi1 > _jointLimits[0][1] || phi2 < _jointLimits[1][0] || phi2 > _jointLimits[1][1]){
		Serial.println(F("Switching to other configuration"));
		phi2 = - convertToDeg(phi2n + beta_new);
		phi1 = - convertToDeg(gamma - alpha);
	}
	
	// Calculate angle for 3rd axis (z-direction)
	z = z - _tcpOffset[2];
	float phi3 = -1 * z * gearRatio;	// Multiply by -1 since negative values mean that axis moves in
	
	// Check if angles are valid
	if (!checkIfAnglesValid(phi1, phi2, phi3)) return false;
	
	_goalAngles[0] = phi1;
	_goalAngles[1] = phi2;
	_goalAngles[2] = phi3;
	
	return true;
}

void morobot_s_rrp::updateTCPpose(bool output){
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
    float xncn = c_new*cos(actAngles[0] + actAngles[1] + beta_new);
    float yncn = c_new*sin(actAngles[0] + actAngles[1] + beta_new);
    
	_actPos[0] = a + xnb + xncn;
    _actPos[1] = ynb + yncn;
	_actPos[2] = -1 * convertToDeg(actAngles[2])/gearRatio + _tcpOffset[2]; 	// Multiply by -1 since moving in positive z-axis means that the linear axis moves in
	
	// Calculate orientation
	_actOri[0] = 0;
	_actOri[1] = 0;
	_actOri[2] = convertToDeg(actAngles[0] + actAngles[1]);
	
	if (output == true)	printTCPpose();
	
	_tcpPoseIsValid = true;
}