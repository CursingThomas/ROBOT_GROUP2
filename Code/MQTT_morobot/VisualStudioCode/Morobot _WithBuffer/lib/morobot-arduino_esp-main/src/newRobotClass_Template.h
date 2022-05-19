/**
 *  \class 	newRobotClass_Template
 *  \brief 	morobot child class for ADD ROBOT TYPE for microcontrollers such as Arduino or ESP32
 *  @file 	newRobotClass_Template.h
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

#ifndef NEWROBOTCLASS_TEMPLATE_H
#define NEWROBOTCLASS_TEMPLATE_H

#include <morobot.h>

class newRobotClass_Template:public morobotClass {
	public:
		/**
		 *  \brief Constructor of newRobotClass_Template class
		 *  \details The value in brakets defines the number of smart servo motors
		 */
		newRobotClass_Template() : morobotClass(3){memcpy(_robotJointLimits, _jointLimits, 3*2*sizeof(long)); memcpy(_robotAxisLimits, _axisLimits, 3*2*sizeof(uint8_t));};	// TODO: PUT THE NUMBER OF SERVOS HERE
		
		/**
		 *  \brief Set the position of the TCP (tool center point) with respect to the center of the flange of the last robot axis.
		 *  		This information is necessary to calculate the inverse kinematics correctly.
		 *  		The function stores the TCP-Offset and recalculates the length and angle of the last axis.
		 *  		Internally the length and angle of the last axis is stored as if it would be a straigth axis directly to the TCP.
		 *  \param [in] xOffset Offset in x-direction
		 *  \param [in] yOffset Offset in y-direction
		 *  \param [in] zOffset Offset in z-direction
		 */
		virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
		
		/**
		 *  \brief Checks if a given angle can be reached by the joint. Each joint has a specific limit to protect the robot's mechanics.
		 *  		The joint limits are predefined in the private variable _jointLimits
		 *  \param [in] servoId Number of motor to move (first motor has ID 0)
		 *  \param [in] angle Angle to move the robot to in degrees
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		virtual bool checkIfAngleValid(uint8_t servoId, float angle);

		/**
		 *  \brief Returns the type of the robot
		 *  \return Returns morobot type
		 */
		virtual String getType();		
		
		const String type = "newRobotClass_Template";		//!< Type of morobot

	protected:
		/**
		 *  \brief Uses given coordinates to calculate the motor angles to reach this position (Solve inverse kinematics).
		 *			This function does only calculate the angles of the motors and stores them internally.
		 *  		Use moveToPose(x,y,z) to actually move the robot.
		 *  \param [in] x Desired x-position of TCP
		 *  \param [in] y Desired x-position of TCP
		 *  \param [in] z Desired x-position of TCP
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		virtual bool calculateAngles(float x, float y, float z);
		
		/**
		 *  \brief Re-calculates the internally stored robot TCP position (Solves forward kinematics).
		 *  		This function does calculate and store the TCP position depending on the current motor angles.
		 */
		virtual void updateTCPpose(bool output = false);

	private:
		float _tcpOffset[3];	//!< Position of the TCP (tool center point) with respect to the center of the flange of the last robot axis
		long _jointLimits[3][2] = {{-100, 100}, {-100, 100}, {0, 780}};		//!< Limits for all joints
		uint8_t _axisLimits[3][2] = {{-100, 100}, {-100, 100}, {-50, 50}};	//!< Limits of x, y, z axis
				// TODO: CHANGE THE NUMBER OF SERVOS AND THE LIMITS HERE
		
		//TODO: PUT VARIABLES FOR INVERSE KINEMATICS HERE
};

#endif