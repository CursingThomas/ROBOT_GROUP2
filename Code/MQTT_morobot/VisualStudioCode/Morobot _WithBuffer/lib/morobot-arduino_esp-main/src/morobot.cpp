/**
 *  \class 	morobotClass
 *  \brief 	morobot base class for microcontrollers such as Arduino or ESP32
 *  @file 	morobot.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
 *  		morobotClass(uint8_t numSmartServos);
			void begin(const char* stream);
			void setZero();
			void moveHome();
			void setSpeedRPM(uint8_t speed);
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
			
			void setBreaks();
			void releaseBreaks();

			void setBusy();
			void setIdle();
			void waitUntilIsReady();
			bool checkIfMotorMoves(uint8_t servoId);
			
			long getActAngle(uint8_t servoId);
			float getActPosition(char axis);
			float getActOrientation(char axis);
			float getSpeed(uint8_t servoId);
			float getTemp(uint8_t servoId);
			float getVoltage(uint8_t servoId);
			float getCurrent(uint8_t servoId);
			long getJointLimit(uint8_t servoId, bool limitNum);
			uint8_t getAxisLimit(char axis, bool limitNum);
			uint8_t getNumSmartServos();
			
			void moveToAngle(uint8_t servoId, long angle);
			void moveToAngle(uint8_t servoId, long angle, uint8_t speedRPM, bool checkValidity=true);
			void moveToAngles(long angles[]);
			void moveToAngles(long angles[], uint8_t speedRPM);
			void moveAngle(uint8_t servoId, long angle);
			void moveAngle(uint8_t servoId, long angle, uint8_t speedRPM, bool checkValidity=true);
			void moveAngles(long angles[]);
			void moveAngles(long angles[], uint8_t speedRPM);
			bool moveToPose(float x, float y, float z);
			bool moveXYZ(float xOffset, float yOffset, float zOffset);
			bool moveInDirection(char axis, float value);
			
			void printAngles(long angles[]);
			void printTCPpose();
			float convertToDeg(float angle);
			float convertToRad(float angle);
			virtual String getType();
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			virtual void updateTCPpose(bool output);
			void autoCalibrateLinearAxis(uint8_t servoId, uint8_t maxMotorCurrent=25);
		private:
			bool isReady();
 */

#include "morobot.h"

morobotClass::morobotClass(uint8_t numSmartServos){
	if (numSmartServos > NUM_MAX_SERVOS){
		Serial.print(F("Too many motors! Maximum number of motors: "));
		Serial.println(NUM_MAX_SERVOS);
	}
	_numSmartServos = numSmartServos;
}

void morobotClass::begin(const char* stream){
	Serial.begin(115200);
	#if defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
		if (stream == "Serial") {
			Serial.println(F("WARNING: Serial on Arduino Mega is connected to the USB-Controller, so you may get strange bytestings in the serial monitor!"));
			_port = &Serial;
		} else if (stream == "Serial1") {
			Serial1.begin(115200);
			_port = &Serial1;
		} else if (stream == "Serial2") {
			Serial2.begin(115200);
			_port = &Serial2;
		} else if (stream == "Serial3") {
			Serial3.begin(115200);
			_port = &Serial3;
		} else {
			Serial.println(F("ERROR: Serial-Parameter not valid. Choose 'Serial', 'Serial1', 'Serial2' or 'Serial3'."));
		}
		
	#elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MINI) || defined (ARDUINO_AVR_NANO)
		if (stream == "Serial") {
			Serial.println(F("WARNING: Serial on Arduino UNO is connected to the USB-Controller, so you may get strange bytestings in the serial monitor!"));
			_port = &Serial;
		} else {
			Serial.println(F("ERROR: Serial-Parameter not valid. Only 'Serial' possible."));
		}
		
	#elif defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_MICRO) || defined(ARDUINO_AVR_YUN)
		if (stream == "Serial1") {
			Serial1.begin(115200);
			_port = &Serial1;
		} else {
			Serial.println(F("ERROR: Serial-Parameter not valid. Only 'Serial1' possible."));
		}
		
	#elif defined(ESP32)
		if (stream == "Serial") {
			Serial.println(F("WARNING: Serial on ESP32 is connected to the USB-Controller, so you may get strange bytestings in the serial monitor!"));
			_port = &Serial;
		} else if (stream == "Serial1") {
			Serial1.begin(115200, SERIAL_8N1, 18, 19);		// Map the serial pins to different pins since 9/10 are not mapped
			_port = &Serial1;
		} else if (stream == "Serial2") {
			Serial2.begin(115200);
			_port = &Serial2;
		} else {
			Serial.println(F("ERROR: Serial-Parameter not valid. Choose 'Serial1' or 'Serial2'."));
		}
		
	#elif defined(ESP8266)
		if (stream == "Serial") {
			Serial.println(F("WARNING: Serial on ESP32 is connected to the USB-Controller, so you may get strange bytestings in the serial monitor!"));
			_port = &Serial;
		} else {
			Serial.println(F("ERROR: Serial-Parameter not valid. Only 'Serial' possible."));
		}	
	#else
		#error "Board not supported"
	#endif
		
	smartServos.beginSerial(_port);
	delay(5);
	smartServos.assignDevIdRequest();
	delay(50);
	
	setTCPoffset(0, 0, 0);
	setSpeedRPM(25);
	updateTCPpose();

	Serial.println(F("Morobot initialized. Connection to motors established"));
}

void morobotClass::setZero(){
	for (uint8_t i=0; i<_numSmartServos; i++) smartServos.setZero(i+1);
	_tcpPoseIsValid = false;
}

void morobotClass::moveHome(){
	for (uint8_t i=0; i<_numSmartServos; i++) smartServos.setInitAngle(i+1, 0, 15);
	waitUntilIsReady();
	_tcpPoseIsValid = false;
}

void morobotClass::setSpeedRPM(uint8_t speed){
	_speedRPM = speed;
	
	// If the speed is bigger than the maximum speed, set it to maximum.
	// If the speed is smaller than then 1, set it to minimum.
	if (speed > SERVO_MAX_SPEED_RPM) _speedRPM = SERVO_MAX_SPEED_RPM;
	if (speed < 0) _speedRPM = 1;
}


/* BREAKS */
void morobotClass::setBreaks(){
	for (uint8_t i=0; i<_numSmartServos; i++) smartServos.setBreak(i+1, BREAK_BRAKED);
}

void morobotClass::releaseBreaks(){
	for (uint8_t i=0; i<_numSmartServos; i++) smartServos.setBreak(i+1, BREAK_LOOSE);
	_tcpPoseIsValid = false;
}


/* ROBOT STATUS */
void morobotClass::setBusy(){
	for (uint8_t i=0; i<_numSmartServos; i++) _angleReached[i] = false;
}

void morobotClass::setIdle(){
	for (uint8_t i=0; i<_numSmartServos; i++) _angleReached[i] = true;
}

void morobotClass::waitUntilIsReady(){
	if (!waitAfterEachMove) {
		setIdle();
		return;
	}
	setBusy();
	unsigned long startTime = millis();
	while (true){
		// Check if the robot is ready yet
		if (isReady() == true) break;
		// Stop waiting if the robot is not ready after a timeout occurs
		if ((millis() - startTime) > TIMEOUT_DELAY) {
			Serial.println(F("TIMEOUT OCCURED WHILE WAITING FOR ROBOT TO FINISH MOVEMENT!"));
			break;
		}
	}
}

bool morobotClass::checkIfMotorMoves(uint8_t servoId){
	long startPos = getActAngle(servoId);
	delay(150);
	if (startPos != getActAngle(servoId)) return true;
	return false;
}


/* GETTERS */
long morobotClass::getActAngle(uint8_t servoId){
	return smartServos.getAngleRequest(servoId+1);
}

float morobotClass::getActPosition(char axis){
	updateTCPpose();
	
	if (axis == 'x') return _actPos[0];
	else if (axis == 'y') return _actPos[1];
	else if (axis == 'z') return _actPos[2];
	else Serial.println(F("ERROR! Invalid axis in getActPosition();"));
}

float morobotClass::getActOrientation(char axis){
	updateTCPpose();
	
	if (axis == 'x') return _actOri[0];
	else if (axis == 'y') return _actOri[1];
	else if (axis == 'z') return _actOri[2];
	else Serial.println(F("ERROR! Invalid axis in getActOrientation();"));
}

float morobotClass::getSpeed(uint8_t servoId){
	return smartServos.getSpeedRequest(servoId+1);
}

float morobotClass::getTemp(uint8_t servoId){
	return smartServos.getTempRequest(servoId+1);
}

float morobotClass::getVoltage(uint8_t servoId){
	return smartServos.getVoltageRequest(servoId+1);
}

float morobotClass::getCurrent(uint8_t servoId){
	return smartServos.getCurrentRequest(servoId+1);
}

long morobotClass::getJointLimit(uint8_t servoId, bool limitNum){
	return _robotJointLimits[servoId][limitNum];
}

uint8_t morobotClass::getAxisLimit(char axis, bool limitNum){
	if (axis == 'x') return _robotAxisLimits[0][limitNum];
	else if (axis == 'y') return _robotAxisLimits[1][limitNum];
	else if (axis == 'z') return _robotAxisLimits[2][limitNum];
}

uint8_t morobotClass::getNumSmartServos(){
	return _numSmartServos;
}

/* MOVEMENTS */
void morobotClass::moveToAngle(uint8_t servoId, long angle){
	if (checkIfAngleValid(servoId, angle) == true) {
		smartServos.moveTo(servoId+1, angle, _speedRPM);
		_tcpPoseIsValid = false;
	}
}

void morobotClass::moveToAngle(uint8_t servoId, long angle, uint8_t speedRPM, bool checkValidity){
	if (checkValidity == false) {
		smartServos.moveTo(servoId+1, angle, speedRPM);
		_tcpPoseIsValid = false;
	} else if (checkIfAngleValid(servoId, angle) == true) {
		smartServos.moveTo(servoId+1, angle, speedRPM);
		_tcpPoseIsValid = false;
	}
}

void morobotClass::moveToAngles(long angles[]){
	waitUntilIsReady();
	Serial.print(F("Moving to [deg]: "));
	printAngles(angles);
	
	for (uint8_t i=0; i<_numSmartServos; i++) moveToAngle(i, angles[i]);
}

void morobotClass::moveToAngles(long angles[], uint8_t speedRPM){
	waitUntilIsReady();
	Serial.print(F("Moving to [deg]: "));
	printAngles(angles);
	
	for (uint8_t i=0; i<_numSmartServos; i++) moveToAngle(i, angles[i], speedRPM);
}

void morobotClass::moveToAngles(long phi0, long phi1, long phi2){
	long angles[3] = {phi0, phi1, phi2};
	moveToAngles(angles);
}

void morobotClass::moveAngle(uint8_t servoId, long angle){
	if (checkIfAngleValid(servoId, getActAngle(servoId)+angle) == true) {
		smartServos.move(servoId+1, angle, _speedRPM);
		_tcpPoseIsValid = false;
	}
}

void morobotClass::moveAngle(uint8_t servoId, long angle, uint8_t speedRPM, bool checkValidity){
	if (checkValidity == false) {
		smartServos.move(servoId+1, angle, speedRPM);
		_tcpPoseIsValid = false;
	} else if (checkIfAngleValid(servoId, getActAngle(servoId)+angle) == true) {
		smartServos.move(servoId+1, angle, speedRPM);
		_tcpPoseIsValid = false;
	}
}

void morobotClass::moveAngles(long angles[]){
	waitUntilIsReady();
	Serial.print(F("Moving [deg]: "));
	printAngles(angles);

	for (uint8_t i=0; i<_numSmartServos; i++) moveAngle(i, angles[i]);
}

void morobotClass::moveAngles(long angles[], uint8_t speedRPM){
	waitUntilIsReady();
	Serial.print(F("Moving [deg]: "));
	printAngles(angles);

	for (uint8_t i=0; i<_numSmartServos; i++) moveAngle(i, angles[i], speedRPM);
}

bool morobotClass::moveToPose(float x, float y, float z){
	waitUntilIsReady();
	Serial.print(F("Moving to [mm]: "));
	Serial.print(x);
	Serial.print(", ");
	Serial.print(y);
	Serial.print(", ");
	Serial.println(z);
	
	updateTCPpose();
	if (calculateAngles(x, y, z) == false) return false;
	
	for (uint8_t i=0; i<_numSmartServos; i++) moveToAngle(i, _goalAngles[i]);
	
	// Update TCP-Pose
	_actPos[0] = x;
	_actPos[1] = y;
	_actPos[2] = z;
	_tcpPoseIsValid = true;
	
	return true;
}

bool morobotClass::moveXYZ(float xOffset, float yOffset, float zOffset){
	updateTCPpose();
	return moveToPose(_actPos[0]+xOffset, _actPos[1]+yOffset, _actPos[2]+zOffset);
}

bool morobotClass::moveInDirection(char axis, float value){
	updateTCPpose();
	float goalxyz[3];
	goalxyz[0] = _actPos[0];
	goalxyz[1] = _actPos[1];
	goalxyz[2] = _actPos[2];
	
	if (axis == 'x') goalxyz[0] = goalxyz[0]+value;
	else if (axis == 'y') goalxyz[1] = goalxyz[1]+value;
	else if (axis == 'z') goalxyz[2] = goalxyz[2]+value;
	return moveToPose(goalxyz[0], goalxyz[1], goalxyz[2]);
}

/* HELPER */
void morobotClass::printAngles(long angles[]){
	for (uint8_t i=0; i<_numSmartServos; i++) {
		Serial.print(angles[i]);
		if (i != _numSmartServos-1) Serial.print(", ");
		else Serial.println(" ");
	}
}

void morobotClass::printTCPpose(){
	updateTCPpose();
	Serial.print(F("TCP-Pose x, y, z [mm]: "));
	Serial.print(_actPos[0]);
	Serial.print(", ");
	Serial.print(_actPos[1]);
	Serial.print(", ");
	Serial.print(_actPos[2]);
	Serial.print(F("; Orientation around z-axis [degrees]: "));
	Serial.println(_actOri[2]);
}

float morobotClass::convertToDeg(float angle){
	return angle*180/M_PI;
}

float morobotClass::convertToRad(float angle){
	return angle*M_PI/180;
}

/* PROTECTED */
void morobotClass::autoCalibrateLinearAxis(uint8_t servoId, uint8_t maxMotorCurrent){
	while(true){
		moveAngle(servoId, -2, 1, false);
		if (getCurrent(servoId) > 25) break;
	}
	smartServos.setZero(servoId+1);
	Serial.println(F("Linear axis set zero!"));
}

bool morobotClass::checkForNANerror(uint8_t servoId, float angle){
	// The values are NAN if the inverse kinematics does not provide a solution
	if(isnan(angle)){
		Serial.print(F("Angle for motor "));
		Serial.print(servoId);
		Serial.println(F(" is NAN!"));
		_tcpPoseIsValid = false;
		return false;
	}
	return true;
}

void morobotClass::printInvalidAngleError(uint8_t servoId, float angle){
	// Moving the motors out of the joint limits may harm the robot's mechanics
	Serial.print(F("Angle for motor "));
	Serial.print(servoId);
	Serial.print(F(" is invalid! ("));
	Serial.print(angle);
	Serial.println(F(" degrees)."));
	_tcpPoseIsValid = false;
}

/* ROBOT STATUS PRIVATE */
bool morobotClass::isReady(){
	for (uint8_t i=0; i<_numSmartServos; i++) {
		if (_angleReached[i] == false) {
			if (checkIfMotorMoves(i) == false) continue;
			return false;
		}
	}
	return true;
}
