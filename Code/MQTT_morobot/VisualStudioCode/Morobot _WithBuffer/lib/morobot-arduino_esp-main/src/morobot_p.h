/**
 *  \class 	morobot_p
 *  \brief 	morobot child class for morobot-p for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_p.h
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
 *  		morobot_p() : morobotClass(3){};
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
			void moveHome();
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			virtual void updateCurrentXYZ();
 */

#ifndef MOROBOT_P_H
#define MOROBOT_P_H

#include "morobot.h"

class morobot_p:public morobotClass {
	public:
		/**
		 *  \brief Constructor of morobot_p class
		 *  \details The value in brakets defines that the robot consists of three smartservos
		 */
		morobot_p() : morobotClass(3){memcpy(_robotJointLimits, _jointLimits, 3*2*sizeof(long)); memcpy(_robotAxisLimits, _axisLimits, 3*2*sizeof(uint8_t));};
		
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
		 *  \brief Check if the difference between two given angles is in a valid range. If the difference between servo1 and servo2 is too small/big the mechanics can be harmed.
		 *  \param [in] servo1Angle Angle of servo1
		 *  \param [in] servo2Angle Angle of servo2
		 *  \return Returns true if the values are valid; flase if they are not.
		 */
		bool checkIfAngleDiffValid(float servo1Angle, float servo2Angle);
		
		/**
		 *  \brief Alternate home function which does not move the robot to 0/0/0 degrees but to a regular position where it does not push into the desk
		 */
		void moveHome();

		/**
		 *  \brief Returns the type of the robot
		 *  \return Returns morobot type
		 */
		virtual String getType();		
		
		const String type = "morobot_p";		//!< Type of morobot
		
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
		long _jointLimits[3][2] = {{-360, 360}, {0, 115}, {-100, 28}};		//!< Limits for all joints
		uint8_t _axisLimits[3][2] = {{-300, 300}, {-300, 300}, {50, 210}};	//!< Limits of x, y, z axis
		
		float d1 = 88.20;		//!< Distance between base and rotational axes of motor 2
		float a1 = 120;			//!< Length of first link connected to motor 2
		float a2 = 150;			//!< Length of link between first link and eef-flange
		float a3 = 26.27;		//!< Distance between rotational axis and tcp on eef-flange (forward-direction)

		float z_def_offset = 15;    //!< Distance between rotational axis and tcp on eef-flange (z-direction
		float x_def_offset = 10;    //!< Distance between base-coordinate-frame and rotational axis of motor 1
};

#endif