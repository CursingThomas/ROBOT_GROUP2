/**
 *  \class 	gripper, binaryEEF
 *  \brief 	Endeffector class for all kinds of endeffectors for the morobots
 *  @file 	eef.cpp
 *  @author	Johannes Rauer FHTW
 *  @date	2020/12/22
 *  \par Method List:
 *  	gripper:
			public:
				gripper(morobotClass* morobotToAttachTo);
				void begin();
				void begin(int8_t servoPin);
				bool autoCalibrate();
				void setParams(float degClosed, float degOpen, float degCloseLimit, float degOpenLimit, float gearRatio=7.87, float closingWidthOffset=101.2);
				void setSpeed(uint8_t speed);
				void setSpeed(uint8_t speedOpening, uint8_t speedClosing);
				void setTCPoffset(float xOffset, float yOffset, float zOffset);
				void close();
				void open();
				bool moveAngle(float angleInc, uint8_t speed=50);
				bool moveToAngle(float angle, uint8_t speed=50);
				bool moveWidth(float width, uint8_t speed=50);
				bool moveToWidth(float width, uint8_t speed=50);
				bool closeToForce(float maxCurrent=70);
				float getCurrentOpeningAngle();
				float getCurrentOpeningWidth();
				bool isClosed();
				bool isOpened();
			private:
				bool checkIfAngleValid(float angle);
				bool waitUntilFinished();
				bool functionNotImplementedError();
 *  	binaryEEF:
 *  		public:
 *  			binaryEEF(int8_t pin);
 *  			void activate();
 *  			void deactivate();
 *  			bool isActivated();
 *  			bool isDeactivated();
 */
 
#include "eef.h"

gripper::gripper(morobotClass* morobotToAttachTo){
	_isOpened = false;
	_isClosed = false;
	
	morobot = morobotToAttachTo;
}

void gripper::begin(){
	_gripperType = 0;
	_servoID = morobot->getNumSmartServos();	// e.g. robot has 3 smart servos (ID 0,1,2) -> gripper gets id 3
	
	setSpeed(50);
	setParams(0, -500, 0, -550);	// degClosed, degOpen, degCloseLimit, degOpenLimit
	setTCPoffset(0, 0, -38);		// Store TCP-Offset and change TCP-Offset of morobot

	Serial.println(F("Gripper connected to robot"));
}

void gripper::begin(int8_t servoPin){
	_gripperType = 1;
	_servoID = -1;

	setSpeed(50);
	setParams(65, 140, 50, 155, 1.0, 0);	// values for servomotor -> degClosed is defined as 0 degrees in real life. degClosed, degOpen, degCloseLimit, degOpenLimit, gearRatio, openWidthOffset
	setTCPoffset(0, 0, -18);				// Store TCP-Offset and change TCP-Offset of morobot
	
	servo.attach(servoPin);
	open();

	Serial.println(F("Gripper connected to robot"));
}

bool gripper::autoCalibrate(){
	Serial.println(F("Autocalibrating Gripper"));
	bool returnValue = true;
	
	if (_gripperType == 0){
		Serial.println(F("Closing..."));
		if (closeToForce() == true){
			Serial.println(F("Closed"));
		} else {
			returnValue = false;
		}
	} else {
		returnValue = false;
		functionNotImplementedError();
	}
	if (returnValue == false) {
		Serial.println(F("ERROR: Calibration failed!"));
		return false;
	}
	
	morobot->smartServos.setZero(_servoID+1);
	
	int8_t positiveFactor = 1;	// Check if the gripper is moving positively when closing so the values for opened/closed are correct
	if (_closingDirectionIsPositive == true) positiveFactor = -1;
	setParams(getCurrentOpeningAngle(), getCurrentOpeningAngle() + positiveFactor * 500, getCurrentOpeningAngle(), getCurrentOpeningAngle() + positiveFactor * 550);

	Serial.print(F("Calibration successful! New Limits: Closed at "));
	Serial.print(_degClosed);
	Serial.print(F(", Opened at "));
	Serial.println(_degOpen);
	close();
	return true;
}

void gripper::setParams(float degClosed, float degOpen, float degCloseLimit, float degOpenLimit, float gearRatio, float closingWidthOffset){
	_degClosed = degClosed;
	_degOpen = degOpen;
	_degCloseLimit = degCloseLimit;
	_degOpenLimit = degOpenLimit;
	_gearRatio = gearRatio;
	_closingWidthOffset = closingWidthOffset;
	
	if (_degClosed > _degOpen) _closingDirectionIsPositive = true;
	else _closingDirectionIsPositive = false;

	Serial.print(F("New Limits: Closed at "));
	Serial.print(_degClosed);
	Serial.print(F(", Opened at "));
	Serial.println(_degOpen);	
}

void gripper::setSpeed(uint8_t speed){
	setSpeed(speed, speed);
}

void gripper::setSpeed(uint8_t speedOpening, uint8_t speedClosing){
	_speed[0] = speedOpening;
	_speed[1] = speedClosing;
}

void gripper::setTCPoffset(float xOffset, float yOffset, float zOffset){
	_tcpOffset[0] = xOffset;
	_tcpOffset[1] = yOffset;
	_tcpOffset[2] = zOffset;
	morobot->setTCPoffset(_tcpOffset[0], _tcpOffset[1], _tcpOffset[2]);
}

void gripper::close(){
	float angle = _degClosed;
	if (_gripperType == 1) angle = 0;
	moveToAngle(angle, _speed[1]);
	_isOpened = false;
	_isClosed = true;
}

void gripper::open(){
	float angle = _degOpen;
	if (_gripperType == 1) angle = _degOpen / _gearRatio - _degClosed;	// Calculate from real degrees into motor degrees
	moveToAngle(angle, _speed[0]);
	_isOpened = true;
	_isClosed = false;
}

bool gripper::moveAngle(float angle, uint8_t speed){
	return moveToAngle(getCurrentOpeningAngle() + angle, speed);
}

bool gripper::moveToAngle(float angle, uint8_t speed){
	if (_gripperType == 0){
		if (checkIfAngleValid(angle) != true) return false;			// Check if angle is valid
		float oldAngle = getCurrentOpeningAngle();
		morobot->smartServos.moveTo(_servoID+1, angle, speed);
		_currentAngle = getCurrentOpeningAngle();
		if (abs(oldAngle-angle) > 10) return waitUntilFinished();	// Only wait for bigger movements
	} else if (_gripperType == 1) {
		angle = angle * _gearRatio + _degClosed;					// Calculate angle in motor-degrees
		if (checkIfAngleValid(angle) != true) return false;
		
		float timeForOneDegree = 60 / (float)speed / 360;
		float angleStep = 1;
		float angleTemp = getCurrentOpeningAngle() * _gearRatio + _degClosed;	// Calculate current angle in motor-degrees
		
		bool goalIsBiggerThanCurrent = true;						// Check orientation direction depending on values
		if (angle <= angleTemp) goalIsBiggerThanCurrent = false;
		
		while(1){
			if (goalIsBiggerThanCurrent == true){
				angleTemp = angleTemp + angleStep;
				if (angleTemp > angle) break;
			} else if (goalIsBiggerThanCurrent == false){
				angleTemp = angleTemp - angleStep;
				if (angleTemp < angle) break;
			}
			servo.write(angleTemp);									// Move only a little bit and
			delay(timeForOneDegree * angleStep * 1000);				// wait to stick to defined speed
		}
		_currentAngle = angle / _gearRatio - _degClosed;			// Store new angle (in real degrees)
	}
	_isOpened = false;
	_isClosed = false;
	return true;
}

bool gripper::moveWidth(float width, uint8_t speed){
	if (_gripperType == 0) return moveToWidth(getCurrentOpeningWidth() + width, speed);
	else return functionNotImplementedError();
}

bool gripper::moveToWidth(float width, uint8_t speed){
	if (_gripperType == 0){ 
		int8_t positiveFactor = 1;
		if (_closingDirectionIsPositive == true) positiveFactor = -1;
		
		float angle = positiveFactor * (width - _closingWidthOffset) * _gearRatio;
		moveToAngle(angle, speed);
	} else {
		return functionNotImplementedError();
	}
}

bool gripper::closeToForce(float maxCurrent){
	if (_gripperType == 0){
		float closingStep = -5;
		if (_closingDirectionIsPositive == true) closingStep = closingStep * -1;		// Define rotation direction
		unsigned long startTime = millis();
		while(true){
			morobot->smartServos.move(_servoID+1, closingStep, 20);						// Move the motor a little bit
			if (morobot->smartServos.getCurrentRequest(_servoID+1) > maxCurrent) {		// Check the current
				delay(20);
				if (morobot->smartServos.getCurrentRequest(_servoID+1) > maxCurrent) {	// If after 20ms there is still too much current, the final position is reached (outlier detection)
					Serial.println(F("Grasped object"));
					_currentAngle = getCurrentOpeningAngle();
					_isOpened = false;
					_isClosed = true;
					return true;	
				}
			}
			
			// Stop if the gripper is not finished after a timeout occurs
			if ((millis() - startTime) > TIMEOUT_DELAY_GRIPPER) {
				Serial.println(F("TIMEOUT OCCURED WHILE WAITING FOR GRIPPER TO FINISH MOVEMENT!"));
				return false;
			}
		}
	} else {
		return functionNotImplementedError();
	}
	return false;
}

float gripper::getCurrentOpeningAngle(){
	if (_gripperType == 0) {
		_currentAngle = morobot->smartServos.getAngleRequest(_servoID+1);
		return _currentAngle;
	} else if (_gripperType == 1) {
		return _currentAngle;
	}
	return functionNotImplementedError();
}

float gripper::getCurrentOpeningWidth(){
	if (_gripperType == 0) return _closingWidthOffset + abs(getCurrentOpeningAngle() / _gearRatio);
	else return functionNotImplementedError();
}

bool gripper::isClosed(){
	return _isClosed;
}

bool gripper::isOpened(){
	return _isOpened;
}

bool gripper::checkIfAngleValid(float angle){
	if ((_closingDirectionIsPositive == true && (angle > _degCloseLimit || angle < _degOpenLimit)) || (_closingDirectionIsPositive == false && (angle < _degCloseLimit || angle > _degOpenLimit))){
		Serial.println(F("ERROR: ANGLE OUT OF LIMIT"));
		Serial.println(angle);
		return false;
	}
	return true;
}

bool gripper::waitUntilFinished(){
	unsigned long startTime = millis();
	while (true){
		long startPos = getCurrentOpeningAngle();
		delay(50);
		if (startPos == getCurrentOpeningAngle()) return true;	// If after 50ms the position is still the same, the gripper has reached the goal
		// Stop waiting if the gripper is not finished after a timeout occurs
		if ((millis() - startTime) > TIMEOUT_DELAY_GRIPPER) {
			Serial.println(F("TIMEOUT OCCURED WHILE WAITING FOR GRIPPER TO FINISH MOVEMENT!"));
			return false;
		}
	}
}

bool gripper::functionNotImplementedError(){
	Serial.println(F("ERROR: Function not implemented for this gripper type"));
	return false;
}

/* ************************************************************************************************************************************************************ */

binaryEEF::binaryEEF(int8_t pin){
	pinMode(pin, OUTPUT);
	_pin = pin;
	_isActive = false;
}

void binaryEEF::activate(){
	digitalWrite(_pin, HIGH);
	_isActive = true;
}
void binaryEEF::deactivate(){
	digitalWrite(_pin, LOW);
	_isActive = false;
}

bool binaryEEF::isActivated(){
	return _isActive;
}

bool binaryEEF::isDeactivated(){
	return !_isActive;
}

