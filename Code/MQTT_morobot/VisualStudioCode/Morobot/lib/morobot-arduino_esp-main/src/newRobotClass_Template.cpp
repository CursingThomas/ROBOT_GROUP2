/**
 *  \class 	newRobotClass_Template
 *  \brief 	morobot child class for newRobotClass_Template for microcontrollers such as Arduino or ESP32
 *  @file 	newRobotClass_Template.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
 *  		newRobotClass_Template() : morobotClass(NUM_SERVOS){};
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			virtual void updateCurrentXYZ();
 */
 
#include <newRobotClass_Template.h>

void newRobotClass_Template::setTCPoffset(float xOffset, float yOffset, float zOffset){
	_tcpOffset[0] = xOffset;
	_tcpOffset[1] = yOffset;
	_tcpOffset[2] = zOffset;
	_tcpPoseIsValid = false;
	
	// Calculate new length and angle of last axis (since eef is connected to it statically)
	//TODO: PUT YOUR CODE TO RE-CALCULATE LENGTHS OF ROBOT HERE 
}

bool newRobotClass_Template::checkIfAngleValid(uint8_t servoId, float angle){
	// The values are NAN if the inverse kinematics does not provide a solution
	if(checkForNANerror(servoId, angle)) return false;
	
	// Moving the motors out of the joint limits may harm the robot's mechanics
	if(angle < _jointLimits[servoId][0] || angle > _jointLimits[servoId][1]){
		printInvalidAngleError(servoId, angle);
		return false;
	}
	
	return true;
}

String newRobotClass_Template::getType(){
	return type;
}


/* PROTECTED FUNCTIONS */
bool newRobotClass_Template::calculateAngles(float x, float y, float z){
	//TODO: CALCULATE THE MOTOR VALUES USING THE INVERSE KINEMATICS
	
	//TODO: CHECK IF THE ANGLES ARE VALID E.G.:
	//if (!checkIfAnglesValid(phi1, phi2, phi3)) return false;
	
	//TODO: IF THE ANGLES ARE VALID: STORE YOUR CALCULATED ANGLES E.G.:
	//_goalAngles[0] = phi1;
	
	return true;
}

void newRobotClass_Template::updateTCPpose(bool output){
	if (_tcpPoseIsValid) return;
	
	waitUntilIsReady();
	
	//TODO: SOLVE FORWARD KINEMATICS TO GET TCP POSITION
	
	//TODO: STORE THE POSITION FOR ALL AXES E.G.:
	//_actPos[0] = a + xnb + xncn;
	
	//TODO: STORE THE ORIENTATION OF THE TCP
	//_actOri[0] = 0;
	
	if (output == true) printTCPpose();
}