/**
 *  \class 	morobotClass
 *  \brief 	morobot base class for microcontrollers such as Arduino or ESP32
 *  @file 	morobot.h
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

#include <Arduino.h>
#include "MakeblockSmartServo.h"
#include "eef.h"

#include "morobot_s_rrr.h"
#include "morobot_s_rrp.h"
#include "morobot_2d.h"
#include "morobot_3d.h"
#include "morobot_p.h"

#ifndef MOROBOT_H
#define MOROBOT_H

#define BREAK_LOOSE  1			//!< Defines if a break of a smart-servo is loose
#define BREAK_BRAKED 0			//!< Defines if a break of a smart-servo is set
#define NUM_MAX_SERVOS 10		//!< Maximum number of smart servos usable in one robot object
#define TIMEOUT_DELAY 15000		//!< Delaytime until the robot stops waiting for motors to finish their movement

class morobotClass {
	public:
		/**
		 *  \brief Constructor of morobot class
		 *  \param [in] numSmartServos Number of smart servos of the robot
		 */
		morobotClass(uint8_t numSmartServos);
		
		/**
		 *  \brief Starts the communication with the smartservos of the robot
		 *  \param [in] stream Name of serial port (e.g. "Serial1").
		 *  \details The names of the Serial ports for Arduino controllers can be found here: https://www.arduino.cc/reference/en/language/functions/communication/serial/
		 */
		void begin(const char* stream);
		
		/**
		 *  \brief Sets the current position as origin (zero position)
		 *  \details Call this function after bringing the motors into their initial (zero position) to store it permanently as 0 degrees
		 */
		void setZero();
		
		/**
		 *  \brief Moves all motors to their origin (zero position)
		 *  \details Only call this function after calibration (Setting a zero position for all motors using setZero())
		 */
		void moveHome();
		
		/**
		 *  \brief Sets the speed in RPM (rounds per minute) for all motors. Does not move a motor.
					Use this function to set a default speed for all motors so you do not have to give a speed every time the robot should move.
					If a different speed is given in a movement-function, this speed will be used only for this movement.
		 *  \param [in] speed Desired velocity of the motors in RPM (rounds per minute). Values accepted between 1 and 50.
		 */
		void setSpeedRPM(uint8_t speed);
		
		/**
		 *  \brief Sets the position of the TCP (tool center point) with respect to the center of the flange of the last robot axis.
					Virtual function. Defined individually for each robot type in the respective child classes.
					This information is necessary to calculate the inverse kinematics correctly.
		 *  \param [in] xOffset Offset in x-direction
		 *  \param [in] yOffset Offset in y-direction
		 *  \param [in] zOffset Offset in z-direction
		 */
		virtual void setTCPoffset(float xOffset, float yOffset, float zOffset)=0;
		
		/**
		 *  \brief Checks if a given angle can be reached by the joint. Each joint has a specific limit to protect the robot's mechanics.
		 *  		Virtual function. Defined individually for each robot type in the respective child classes.
		 *  \param [in] servoId Number of motor to move (first motor has ID 0)
		 *  \param [in] angle Angle to move the robot to in degrees
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		virtual bool checkIfAngleValid(uint8_t servoId, float angle)=0;
		
		/* BREAKS */
		/**
		 *  \brief Sets the breaks of all motors. Axes cannot be moved after calling this function.
		 *  \details After moving a motor with a movement-function, the motor is breaked at final position automatically. 
		 */
		void setBreaks();
		
		/**
		 *  \brief Releases the breaks of all motors. Axes can be moved freely after calling this function.
		 *  \details After moving a motor with a movement-function, the motor is breaked at final position automatically.
		 *  		 Use this function to release breaks to move the motor with your hands.
		 */
		void releaseBreaks();

		/* ROBOT STATUS */
		/**
		 *  \brief Sets internal variables which indicate, that the robot should not be moved at the moment.
		 *  \details Use setIdle() to indicate, that the robot can be moved.
		 *  		 Use waitUntilIsReady() to wait until the robot is idle (no motor moves) before continuing.
		 */
		void setBusy();
		
		/**
		 *  \brief Sets internal variables which indicate, that the robot can be moved.
		 *  \details Use setBusy() to indicate, that the robot should not be moved.
		 */
		void setIdle();
		
		/**
		 *  \brief Waits until the robot is ready for new commants (all motors have stopped moving) or a timeout occurs.
		 *  \details Function sets the robot idle only when all motors have stopped moving or a timeout occurs.
		 *  		 If a timeout occurs this is printed to the serial monitor.
		 */
		void waitUntilIsReady();
		
		/**
		 *  \brief Check if a given smart servo is moving at the moment.
		 *  		Function stores current angle of motor, waits some time and compares the angle before and after waiting.
		 *  \param [in] servoId Number of motor to check (first motor has ID 0)
		 *  \return Returns True if motor is moving; False if motor is not moving.
		 */
		bool checkIfMotorMoves(uint8_t servoId);
		
		/* GETTERS */
		/**
		 *  \brief Returns angle-position of motor in degrees.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \return Angle-positon in degrees.
		 */
		long getActAngle(uint8_t servoId);
		
		/**
		 *  \brief Returns position of TCP in mm in given axis (in robot base frame).
		 *  \param [in] axis Possible parameters: 'x', 'y', 'z'
		 *  \return Position of TCP in mm in given axis.
		 */
		float getActPosition(char axis);
		
		/**
		 *  \brief Returns orientation of TCP in degrees in given axis (in robot base frame).
		 *  \param [in] axis of rotation Possible parameters: 'x', 'y', 'z'
		 *  \return Orientation of TCP in degrees in given axis.
		 */
		float getActOrientation(char axis);
		
		/**
		 *  \brief Returns current speed of motor in RPM (rounds per minute).
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \return Current speed of motor in RPM.
		 */
		float getSpeed(uint8_t servoId);
		
		/**
		 *  \brief Returns current temperature of motor in degrees Celsius.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \return Current temperature of motor in degrees Celsius.
		 */
		float getTemp(uint8_t servoId);
		
		/**
		 *  \brief Returns current voltage of motor.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \return Current voltage of motor.
		 */
		float getVoltage(uint8_t servoId);
		
		/**
		 *  \brief Returns current current consumption of motor in Ampere.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \return Current current consumption of motor in Ampere.
		 */
		float getCurrent(uint8_t servoId);

		/**
		 *  \brief Returns the limits of a given axis
		 *  \param [in] servoId Number of servo to get limit of
		 *  \param [in] limitNum 0 for lower limit, 1 for upper limit
		 */
		long getJointLimit(uint8_t servoId, bool limitNum);
		
		/**
		 *  \brief Return the limit in x/y/z orientation
		 *  \param [in] axis Axis for which to get the limit
		 *  \param [in] limitNum 0 for lower limit, 1 for upper limit
		 */
		 uint8_t getAxisLimit(char axis, bool limitNum);
		
		/**
		 * \brief Returns number of smart servos in robot 
		 * \return Returns number of smart servos in robot
		 */
		uint8_t getNumSmartServos();
		
		/* MOVEMENTS */
		/**
		 *  \brief Moves a motor to a desired angle (absolute movement).
		 *  		Checks if the angle is valid before moving if checkValidity is not given or true.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \param [in] angle Desired goal angle in degrees.
		 */
		void moveToAngle(uint8_t servoId, long angle);
		
		/**
		 *  \brief Moves a motor to a desired angle (absolute movement).
		 *  		Checks if the angle is valid before moving if checkValidity is not given or true.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \param [in] angle Desired goal angle in degrees.
		 *  \param [in] speedRPM Desired velocity of the motor in RPM (rounds per minute). Values accepted between 1 and 50. If no speed is given, the preset default speed is used.
		 *  \param [in] checkValidity (Optional) Set this variable to "false" if you don't want to check if the angle value is valid (e.g. necessary for calibration)
		 */
		void moveToAngle(uint8_t servoId, long angle, uint8_t speedRPM, bool checkValidity=true);
		
		/**
		 *  \brief Moves all motors to desired angles (Moves the whole robot) - absolute movement.
					Waits until the robot is ready to use (no motor moves) before moving.
					Checks if the angles are valid before moving.
		 *  \param [in] angles[] Desired goal angles in degrees.
		 */
		void moveToAngles(long angles[]);

		/**
		 *  \brief Moves all motors to desired angles (Moves the whole robot) - absolute movement.
					Waits until the robot is ready to use (no motor moves) before moving.
					Checks if the angles are valid before moving.
		 *  \param [in] angles[] Desired goal angles in degrees.
		 *  \param [in] speedRPM Desired velocity of the motor in RPM (rounds per minute). Values accepted between 1 and 50. If no speed is given, the preset default speed is used.
		 */
		void moveToAngles(long angles[], uint8_t speedRPM);

		/**
		 *  \brief Moves all motors to desired angles (Moves the whole robot) - absolute movement. Since most morobots have three motors, this works without an array
					Waits until the robot is ready to use (no motor moves) before moving.
					Checks if the angles are valid before moving.
		 *  \param [in] phi0 Desired goal angle for motor 0
		 *  \param [in] phi1 Desired goal angle for motor 1
		 *  \param [in] phi2 Desired goal angle for motor 2
		 */
		void moveToAngles(long phi0, long phi1, long phi2);

		/**
		 *  \brief Moves a motor by a desired angle (relative movement).
		 *  		Checks if the goal angle is valid before moving if checkValidity is not given or true.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \param [in] angle Desired angle in degrees to move robot by.
		 */
		void moveAngle(uint8_t servoId, long angle);
		
		/**
		 *  \brief Moves a motor by a desired angle (relative movement).
		 *  		Checks if the goal angle is valid before moving if checkValidity is not given or true.
		 *  \param [in] servoId Number of motor (first motor has ID 0)
		 *  \param [in] angle Desired angle in degrees to move robot by.
		 *  \param [in] speedRPM Desired velocity of the motor in RPM (rounds per minute). Values accepted between 1 and 50. If no speed is given, the preset default speed is used.
		 *  \param [in] checkValidity (Optional) Set this variable to "false" if you don't want to check if the angle value is valid (e.g. necessary for calibration)
		 */
		void moveAngle(uint8_t servoId, long angle, uint8_t speedRPM, bool checkValidity=true);
		
		/**
		 *  \brief Moves all motors by desired angles (Moves the whole robot) - relative movement.
					Waits until the robot is ready to use (no motor moves) before moving.
		 *  		Checks if the angles are valid before moving.
		 *  \param [in] angles[] Desired angles in degrees to move robot by.
		 */
		void moveAngles(long angles[]);
		
		/**
		 *  \brief Moves all motors by desired angles (Moves the whole robot) - relative movement.
					Waits until the robot is ready to use (no motor moves) before moving.
		 *  		Checks if the angles are valid before moving.
		 *  \param [in] angles[] Desired angles in degrees to move robot by.
		 *  \param [in] speedRPM Desired velocity of the motor in RPM (rounds per minute). Values accepted between 1 and 50. If no speed is given, the preset default speed is used.
		 */
		void moveAngles(long angles[], uint8_t speedRPM);
		
		/**
		 *  \brief Moves the TCP (tool center point) of the robot to a desired position.
		 * 			Calls child class to solve inverse kinematics and moves the robot to the position.
		 *			For morobot-s (rrp) the parameter "z" is not the z-position but the rotation around the z-axis in degrees!
		 *  \param [in] x Desired x-coordinate of the TCP in mm (in base frame)
		 *  \param [in] y Desired y-coordinate of the TCP in mm (in base frame)
		 *  \param [in] z Desired z-coordinate of the TCP in mm (in base frame)
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		bool moveToPose(float x, float y, float z);
		
		/**
		 *  \brief Moves the TCP (tool center point) of the robot by given axis-values.
		 *  		Calls child class to solve forward kinematics, adds values, solves inverse kinematics and moves the robot to the position.
		 *  \param [in] xOffset Desired x-value to move the TCP by in mm
		 *  \param [in] yOffset Desired y-value to move the TCP by in mm
		 *  \param [in] zOffset Desired z-value to move the TCP by in mm
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		bool moveXYZ(float xOffset, float yOffset, float zOffset);
		
		/**
		 *  \brief Moves the TCP (tool center point) of the robot by given value in one axis.
		 *  		Calls child class to solve forward kinematics, adds values, solves inverse kinematics and moves the robot to the position.
		 *  \param [in] axis Axis to move the robot in. Possible parameters: 'x', 'y', 'z'
		 *  \param [in] value Value by which the robot should be moved in mm
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		bool moveInDirection(char axis, float value);
		
		/* HELPER */
		/**
		 *  \brief Prints an array of angles to the serial monitor.
		 *  \param [in] angles[] Angle values to print.
		 */
		void printAngles(long angles[]);
		
		/**
		 *  \brief Prints an the TCP-pose to the serial monitor.
		 */
		void printTCPpose();
		
		/**
		 *  \brief Returns angle converted from rad into deg
		 *  \param [in] angle Angle to convert in radians
		 *  \return Returns angle in degree
		 */
		float convertToDeg(float angle);
		
		/**
		 *  \brief Returns angle converted from deg into rad
		 *  \param [in] angle Angle to convert in degrees
		 *  \return Returns angle in radians
		 */
		float convertToRad(float angle);
		
		/**
		 *  \brief Returns the type of the robot
		 *  \return Returns morobot type
		 */
		virtual String getType()=0;
		
		/* PUBLIC VARIABLES */
		MakeblockSmartServo smartServos;	//!< Makeblock smartservo object
		bool waitAfterEachMove = true;		//!< Defines if the robot waits after moving or does not wait until movement has finished
		
	protected:
		/**
		 *  \brief Uses given coordinates to calculate the motor angles to reach this position (Solve inverse kinematics).
		 *  		Virtual function. Defined individually for each robot type in the respective child classes.
		 *  		This function does only calculate the angles of the motors and stores them internally.
		 *  		Use moveToPose(x,y,z) to actually move the robot.
		 *  \param [in] x Desired x-position of TCP
		 *  \param [in] y Desired x-position of TCP
		 *  \param [in] z Desired x-position of TCP
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		virtual bool calculateAngles(float x, float y, float z)=0;

		/**
		 *  \brief Re-calculates the internally stored robot TCP position (Solves forward kinematics).
		 *  		Virtual function. Defined individually for each robot type in the respective child classes.
		 *  		This function does calculate and store the TCP position depending on the current motor angles.
		 */
		virtual void updateTCPpose(bool output = false)=0;
		
		/**
		 *  \brief Calibrates a linear axis by increasing the angle until a current limit is reached
		 *  \param [in] servoId Number of motor to calibrate (first motor has ID 0)
		 *  \param [in] maxMotorCurrent (Optional) Current limit at which zero position is reached an calibration stops
		 */
		void autoCalibrateLinearAxis(uint8_t servoId, uint8_t maxMotorCurrent=80);

		/**
		 *  \brief Checks if a given angle is NAN-value and prints an error message. The values are NAN if the inverse kinematics does not provide a solution.
		 *  \param [in] servoId Number of motor the angle is for (first motor has ID 0)
		 *  \param [in] angle Value to check for NAN
		 *  \return Returns true if the value is valid; false if it is not.
		 */
		bool checkForNANerror(uint8_t servoId, float angle);
		
		/**
		 *  \brief Prints an error message to the console when trying to move a motor out of its valid range
		 *  \param [in] servoId Number of motor which is to be moved (first motor has ID 0)
		 *  \param [in] angle Angle to move the motor to
		 */
		void printInvalidAngleError(uint8_t servoId, float angle);

		uint8_t _numSmartServos;			//!< Number of smart servos of robot
		long _robotJointLimits[3][2];	//!< Limits for all joints
		uint8_t _robotAxisLimits[3][2];		//!< Limits of x, y, z axis
		uint8_t _speedRPM;					//!< Default speed (used if movement-funtions to not provide specific speed)
		float _actPos[3];					//!< Robot TCP position (in base frame)
		float _actOri[3];					//!< Robot TCP orientation (rotation in degrees around base frame)
		bool _tcpPoseIsValid = false;		//!< Status of TCP-pose: When the robot is moved without updating pose, it is set to false;
		bool _angleReached[NUM_MAX_SERVOS];	//!< Variables that indicate if a motor is busy (is moving and has not reached final position)
		float _goalAngles[NUM_MAX_SERVOS];	//!< Variable for inverse kinematics to store goal Angles of the motors
		Stream* _port;						//!< Port used for communication with the robot (e.g. Serial1)
	private:
		/**
		 *  \brief Checks if the robot is busy or idle.
		 *  		Checks if internal variables indicate the robot is idle.
		 *  		If a motor is not already set idle, it checks if the motor is still moving.
		 *  \return Returns true if the robot is idle; false if the robot is busy
		 */
		bool isReady();
};

#endif