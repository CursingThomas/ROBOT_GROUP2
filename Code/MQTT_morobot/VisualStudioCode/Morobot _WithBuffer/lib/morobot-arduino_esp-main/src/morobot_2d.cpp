/**
 *  \class 	morobot_2d
 *  \brief 	morobot child class for morobot-2d for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_2d.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2021/02/17
 *  \par Method List:
 *  	public:
 *  		newRobotClass() : morobotClass(PUT_NUM_SERVOS_HERE){};
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			virtual void updateTCPpose(bool output);
 */
 
#include "morobot_2d.h"

void morobot_2d::setTCPoffset(float xOffset, float yOffset, float zOffset){
	// Add given tcp-offset and default offsets
	_tcpOffset[0] = xOffset;
	_tcpOffset[1] = yOffset;
	_tcpOffset[2] = zOffset;
	_tcpPoseIsValid = false;
}

bool morobot_2d::checkIfAngleValid(uint8_t servoId, float angle){
	// The values are NAN if the inverse kinematics does not provide a solution
	if(!checkForNANerror(servoId, angle)) return false;
	
	// Moving the motors out of the joint limits may harm the robot's mechanics
	if(angle < _jointLimits[servoId][0] || angle > _jointLimits[servoId][1]){
		printInvalidAngleError(servoId, angle);
		return false;
	}
	
	return true;
}

String morobot_2d::getType(){
	return type;
}

/* PROTECTED FUNCTIONS */
bool morobot_2d::calculateAngles(float x, float y, float z){
	// Subtract offset
	x = x - _tcpOffset[0] - x_def_offset;
	y = y - _tcpOffset[1] - y_def_offset;
	z = z - _tcpOffset[2] - z_def_offset_bottom - z_def_offset_top;
	
	// Calculate phi1
	float L_M1_sq = pow(z,2) + pow(x,2);
	float gamma = M_PI/2 - atan2(z,x);
	float alpha = acos((L_M1_sq + pow(L1,2) - pow(L2,2)) / (2 * sqrt(L_M1_sq) * L1));
	float phi1 = alpha - gamma;

	// Calculate phi2
	float L_M2_sq = pow(z,2) + pow(L4-L3-x,2);
	float delta = M_PI/2 - atan2(z, L4-L3-x);
	float beta = acos((L_M2_sq + pow(L1,2) - pow(L2,2)) / (2 * sqrt(L_M2_sq) * L1));
	float phi2 = beta - delta;
	
	// Recalculate for motor mounting orientations in robot
	phi1 = -1 * convertToDeg(phi1) + 90;
	phi2 = convertToDeg(phi2) - 90;
	
	// Check if angles are valid
	if(checkIfAngleValid(0, phi1) == false) return false;
	if(checkIfAngleValid(1, phi2) == false) return false;
	
	_goalAngles[0] = phi1;
	_goalAngles[1] = phi2;
	
	return true;
}

void morobot_2d::updateTCPpose(bool output){
	if (_tcpPoseIsValid) return;
	
	waitUntilIsReady();
	
	// Recalculate angles because of motor mounting orientations
	float phi1 = - (getActAngle(0) - 90);
	float phi2 = getActAngle(1) + 90;
	phi1 = convertToRad(phi1);
	phi2 = convertToRad(phi2);
	
	// Calculate positions of rotation axes
	float Ax = -L1 * sin(phi1);
	float Az = L1 * cos(phi1);
	float Bx = L1 * sin(phi2) + L4 - L3;
	float Bz = L1 * cos(phi2);
	
	// Calculate helping angles
	float L_AB_sq = pow(Bz - Az, 2) + pow(Bx - Ax, 2);
	float alpha = atan2(Bz-Az, Bx-Ax);
	float beta = acos (sqrt(L_AB_sq) / (2 * L2));
	float gamma = alpha + beta;
	
	// Calculate and store position
	_actPos[0] = Ax + L2 * cos(gamma) + L3/2 + _tcpOffset[0] + x_def_offset;
	_actPos[1] = _tcpOffset[1] + y_def_offset;
	_actPos[2] = Az + L2 * sin(gamma) + _tcpOffset[2] + z_def_offset_bottom + z_def_offset_top;

	// Store orientation
	_actOri[0] = 0;
	_actOri[1] = 0;
	_actOri[2] = 0;

	if (output == true)	printTCPpose();
	
	_tcpPoseIsValid = true;
}