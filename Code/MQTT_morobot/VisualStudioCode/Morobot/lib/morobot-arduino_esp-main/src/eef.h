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

#if defined(ESP32)
	#include <ESP32Servo.h>
#else
	#include <Servo.h>
#endif


#ifndef EEF_H
#define EEF_H

#include "morobot.h"

#define TIMEOUT_DELAY_GRIPPER 25000		//!< Delay after which the gripper stops moving when closeToForce() is used

class gripper {
	public:
		/**
		 *  \brief Constructor of gripper class
		 *  \param [in] morobotToAttachTo Pointer to morobot object the gripper is attached to (relevant for smart servo control)
		 */
		gripper(morobotClass* morobotToAttachTo);
		
		/**
		 *  \brief Initialices all important variables for smartservo-gripper
		 *  \details This function must be called in the setup-function of the code.
		 *  This function changes the TCP-Position of the morobot
		 */
		void begin();
		
		/**
		 *  \brief Initialices all important variables for servo-gripper
		 *  \param [in] servoPin The pin of the arduino the servo motor is connected to
		 *  \details This function must be called in the setup-function of the code
		 *  This function changes the TCP-Position of the morobot
		 */
		void begin(int8_t servoPin);
		
		/**
		 *  \brief Autonomous calibration for smart servo gripper
		 *  \return Returns true is calibration was successful
		 *  \details Closes gripper until a defined force (current) appears (= closed position), sets the motor zero and adds the default values for an open gripper 
		 */
		bool autoCalibrate();
		
		/**
		 *  \brief Sets all important parameters of the gripper
		 *  \param [in] degClosed Degrees at which the gripper is closed (used when calling close())
		 *  \param [in] degOpen Degrees at which the gripper is opened (used when calling open())
		 *  \param [in] degCloseLimit Gripper will not close above this limit (would harm hardware)
		 *  \param [in] degOpenLimit Gripper will not open above this limit (would harm hardware)
		 *  \param [in] gearRatio The gearRatio defines how many mm (or degrees) the gripper moves per degree motor movement
		 *  \param [in] closingWidthOffset Opening width of the gripper when it is completely closed (for parallel grippers only
		 */
		void setParams(float degClosed, float degOpen, float degCloseLimit, float degOpenLimit, float gearRatio=7.87, float closingWidthOffset=101.2);
		
		/**
		 *  \brief Sets the opening and closing speed of the gripper
		 *  \param [in] speed Opening and closing speed in RPM (max. 50)
		 */
		void setSpeed(uint8_t speed);
		
		/**
		 *  \brief Sets the opening and closing speed of the gripper independently
		 *  \param [in] speedOpening Opening speed in RPM (max. 50)
		 *  \param [in] speedClosing Closing speed in RPM (max. 50)
		 */
		void setSpeed(uint8_t speedOpening, uint8_t speedClosing);
		
		/**
		 *  \brief Sets the TCP-Offset of the gripper and therefore also of the morobot
		 *  \param [in] xOffset Offset in direction x
		 *  \param [in] yOffset Offset in direction y
		 *  \param [in] zOffset Offset in direction z
		 */
		void setTCPoffset(float xOffset, float yOffset, float zOffset);
		
		/**
		 *  \brief Moves the gripper to its closing position (defined by _degClosed in setParams())
		 */
		void close();
		
		/**
		 *  \brief Moves the gripper to its opening position (defined by _degOpened in setParams())
		 */
		void open();
		
		/**
		 *  \brief Moves the gripper relatively by a defined angle
		 *  \param [in] angleInc Angle to move the gripper by
		 *  \param [in] speed Speed to use for movement (Default: maximum speed)
		 *  \return Returns true if movement has been successful
		 */
		bool moveAngle(float angleInc, uint8_t speed=50);
		
		/**
		 *  \brief Moves the gripper to a defined angle (absolute)
		 *  \param [in] angle Angle to move the gripper to
		 *  \param [in] speed Speed to use for movement (Default: maximum speed)
		 *  \return Returns true if movement has been successful
		 */
		bool moveToAngle(float angle, uint8_t speed=50);
		
		/**
		 *  \brief Moves the gripper relatively by a defined width
		 *  \param [in] width Width to move the gripper by
		 *  \param [in] speed Speed to use for movement (Default: maximum speed)
		 *  \return Returns true if movement has been successful
		 */
		bool moveWidth(float width, uint8_t speed=50);
		
		/**
		 *  \brief Moves the gripper to a defined width (absolute)
		 *  \param [in] width Width to move the gripper to
		 *  \param [in] speed Speed to use for movement (Default: maximum speed)
		 *  \return Returns true if movement has been successful
		 */
		bool moveToWidth(float width, uint8_t speed=50);
		
		/**
		 *  \brief Closes the gripper until a defined force (current) is reached. Only implemented for smart-servo.
		 *  \param [in] maxCurrent Current at which the movement should stop
		 *  \return Returns true if movement has been successful
		 */
		bool closeToForce(float maxCurrent=70);
		
		/**
		 *  \brief Calculates and return current opening angle of the gripper
		 *  \return Current opening angle of the gripper
		 */
		float getCurrentOpeningAngle();

		/**
		 *  \brief Calculates and return current opening width of the gripper. Only implemented for smart-servo (parallel-gripper).
		 *  \return Current opening width of the gripper
		 */
		float getCurrentOpeningWidth();
		
		/**
		 *  \brief Checks if the gripper is closed
		 *  \return True if gripper is closed, false otherwise
		 */
		bool isClosed();
		
		/**
		 *  \brief Checks if the gripper is opened
		 *  \return True if gripper is opened, false otherwise
		 */
		bool isOpened();
		
		morobotClass* morobot;		//!< Pointer to morobot object (for tcp-offset definition and smart-servo-control)
		Servo servo;				//!< Servo-object if a servo-motor is defined
	private:
		/**
		 *  \brief Checks if a given angle can be reached
		 *  \param [in] angle Angle to drive to
		 *  \return True if angle can be reached, false otherwise
		 */
		bool checkIfAngleValid(float angle);
		
		/**
		 *  \brief Makes sure the program does not continue until smart-servo-gripper has stopped moving
		 *  \return True is the gripper reached the target position, false otherwise
		 */
		bool waitUntilFinished();
		
		/**
		 *  \brief Prints a message and return false if the function is not implemented for this gripper-type
		 *  \return Returns false if the function is not implemented for this gripper-type
		 */
		bool functionNotImplementedError();
		
		bool _isClosed;			//!< True if gripper is closed
		bool _isOpened;			//!< True if gripper is opened
		float _currentAngle;	//!< Stores current angle position
		
		uint8_t _gripperType;	//!< 0: smartServo parallel gripper, 1: normal servo
		float _gearRatio;		//!< mm per degree for parallel grippers, real-degrees per motor-degree for other grippers
		bool _closingDirectionIsPositive;	//!< Defines if the motor-angle of the closing angle is bigger than the opening angle (necessary to move the gripper in the correct direction)
		uint8_t _speed[2];		//!< opening-speed, closing-speed
		float _degOpen;			//!< Angle at which the gripper is open (motor-angle)
		float _degClosed;		//!< Angle at which the gripper is closed (motor-angle)
		float _degOpenLimit;	//!< Angle to maximum move the gripper to (motor-angle)
		float _degCloseLimit;	//!< Angle to maximum move the gripper to (motor-angle)
		float _closingWidthOffset;	//!< If gripper is completely closed, it has this gripping width
		float _tcpOffset[3];	//!< Position of the TCP (tool center point) with respect to the center of the flange of the last robot axis
		int8_t _servoID;		//!< ID of the smart-servo
};

class binaryEEF {
	public:
		/**
		 *  \brief Constructor for binary endeffectors (electro-magnets, sucker-cups, ...)
		 *  \param [in] pin Pin to which the endeffector (relais) is connected to
		 */
		binaryEEF(int8_t pin);
		
		/**
		 *  \brief Activates the endeffector
		 */
		void activate();

		/**
		 *  \brief Deactivates the endeffector
		 */
		void deactivate();
		
		/**
		 *  \brief Checks the state of the endeffector
		 *  \return Returns true if the eef is activate, false otherwise
		 */
		bool isActivated();
		
		/**
		 *  \brief Checks the state of the endeffector
		 *  \return Returns false if the eef is activate, true otherwise
		 */
		bool isDeactivated();
	private:
		int8_t _pin;					//!< Pin to which the endeffector (relais) is connected to
		bool _isActive = false;			//!< True if eef is activated, false otherwise
};

#endif