/**
 *  \class 	morobot_3d
 *  \brief 	morobot child class for morobot_3d for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_3d.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			uint8_t calculateAngleYZ(float x, float y, float z, float &theta);
			virtual void updateCurrentXYZ();
 */
 
#include "morobot_3d.h"

void morobot_3d::setTCPoffset(float xOffset, float yOffset, float zOffset){
	// Add given tcp-offset and default offsets
	_tcpOffset[0] = xOffset;
	_tcpOffset[1] = yOffset;
	_tcpOffset[2] = zOffset;
	_tcpPoseIsValid = false;
}

bool morobot_3d::checkIfAngleValid(uint8_t servoId, float angle){
	// The values are NAN if the inverse kinematics does not provide a solution
	if(!checkForNANerror(servoId, angle)) return false;
	
	// Make sure to not get stuck at -0.00 degrees
	if (angle < 0.1 && angle > -0.1) angle = 1;
	
	// Moving the motors out of the joint limits may harm the robot's mechanics
	if(angle < _jointLimits[servoId][0] || angle > _jointLimits[servoId][1]){
		printInvalidAngleError(servoId, angle);
		return false;
	}
	
	return true;
}

String morobot_3d::getType(){
	return type;
}

/* PROTECTED FUNCTIONS */
bool morobot_3d::calculateAngles(float x, float y, float z){
	// Subtract offset
	x = x - _tcpOffset[0];
	y = y - _tcpOffset[1];
	z = z - _tcpOffset[2] - z_def_offset_bottom - z_def_offset_top;
	
	x = x;
	y = -y;
	z = -z;

	float theta1 = 0;
	float theta2 = 0;
	float theta3 = 0;

	// Calculate angles
	uint8_t status = calculateAngleYZ(x, y, z, theta1);
	if (status == 0) status = calculateAngleYZ(x*cos120 + y*sin120, y*cos120-x*sin120, z, theta2);  // rotate coords to +120 deg
	if (status == 0) status = calculateAngleYZ(x*cos120 - y*sin120, y*cos120+x*sin120, z, theta3);  // rotate coords to -120 deg
	if (status != 0) {
		Serial.println(F("ERROR calculating motor angles. The given point is invalid"));
		return false;
	}
	
	theta1 = theta1;
	theta2 = theta2;
	theta3 = theta3;
	
	//Check if angles are valid
	if(checkIfAngleValid(0, theta1) == false) return false;
	if(checkIfAngleValid(1, theta2) == false) return false;
	if(checkIfAngleValid(2, theta3) == false) return false;
	
	_goalAngles[0] = theta1;
	_goalAngles[1] = theta2;
	_goalAngles[2] = theta3;
	
	return true;
}

// Helper functions, calculates angle theta (for YZ-pane)
uint8_t morobot_3d::calculateAngleYZ(float x, float y, float z, float &theta) {
     float y1 = -0.5 * 0.57735 * f; // f/2 * tg 30
     y -= 0.5 * 0.57735    * e;    // shift center to edge
     // z = a + b*y
     float a = (x*x + y*y + z*z +rf*rf - re*re - y1*y1)/(2*z);
     float b = (y1-y)/z;
     // discriminant
     float d = -(a+b*y1)*(a+b*y1)+rf*(b*b*rf+rf); 
     if (d < 0) return -1; // non-existing point
     float yj = (y1 - a*b - sqrt(d))/(b*b + 1); // choosing outer point
     float zj = a + b*yj;
     theta = 180.0*atan(-zj/(y1 - yj))/M_PI + ((yj>y1)?180.0:0.0);
     return 0;
}

void morobot_3d::updateTCPpose(bool output){
	if (_tcpPoseIsValid) return;
	
	waitUntilIsReady();
	
	// Recalculate angles because of motor mounting orientations
	float theta1 = getActAngle(0);
	float theta2 = getActAngle(1);
	float theta3 = getActAngle(2);
	theta1 = convertToRad(theta1);
	theta2 = convertToRad(theta2);
	theta3 = convertToRad(theta3);
	
	float t = (f-e)*tan30/2;

	float y1 = -(t + rf*cos(theta1));
	float z1 = -rf*sin(theta1);

	float y2 = (t + rf*cos(theta2))*sin30;
	float x2 = y2*tan60;
	float z2 = -rf*sin(theta2);

	float y3 = (t + rf*cos(theta3))*sin30;
	float x3 = -y3*tan60;
	float z3 = -rf*sin(theta3);

	float dnm = (y2-y1)*x3-(y3-y1)*x2;

	float w1 = y1*y1 + z1*z1;
	float w2 = x2*x2 + y2*y2 + z2*z2;
	float w3 = x3*x3 + y3*y3 + z3*z3;

	// x = (a1*z + b1)/dnm
	float a1 = (z2-z1)*(y3-y1)-(z3-z1)*(y2-y1);
	float b1 = -((w2-w1)*(y3-y1)-(w3-w1)*(y2-y1))/2.0;

	// y = (a2*z + b2)/dnm;
	float a2 = -(z2-z1)*x3+(z3-z1)*x2;
	float b2 = ((w2-w1)*x3 - (w3-w1)*x2)/2.0;

	// a*z^2 + b*z + c = 0
	float a = a1*a1 + a2*a2 + dnm*dnm;
	float b = 2*(a1*b1 + a2*(b2-y1*dnm) - z1*dnm*dnm);
	float c = (b2-y1*dnm)*(b2-y1*dnm) + b1*b1 + dnm*dnm*(z1*z1 - re*re);

	// discriminant
	float d = b*b - (float)4.0*a*c;
	if (d < 0) {
		Serial.println(F("ERROR: Something went wrong. The calculated TCP pose is no valid point"));
	} else {
		_actPos[2] = -(float)0.5*(b+sqrt(d))/a;
		_actPos[0] = (a1*_actPos[2] + b1)/dnm;
		_actPos[1] = (a2*_actPos[2] + b2)/dnm;
	
		_actPos[0] =   _actPos[0] + _tcpOffset[0];
		_actPos[1] = -_actPos[1] + _tcpOffset[1];
		_actPos[2] = -_actPos[2] + z_def_offset_bottom + z_def_offset_top + _tcpOffset[2];

		// Store orientation
		_actOri[0] = 0;
		_actOri[1] = 0;
		_actOri[2] = 0;

		if (output == true)	printTCPpose();
	}
	_tcpPoseIsValid = true;
}