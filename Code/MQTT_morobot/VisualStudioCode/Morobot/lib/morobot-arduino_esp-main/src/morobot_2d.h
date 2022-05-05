/**
 *  \class 	morobot_2d
 *  \brief 	morobot child class for morobot-2d for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_2d.h
 *  @author	Johannes Rauer FHTW
 *  @date	2021/02/17
 *  \par Method List:
 *  	public:
 *  		morobot_2d() : morobotClass(PUT_NUM_SERVOS_HERE){};
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			virtual void updateTCPpose(bool output = false);
 */

#ifndef MOROBOT_2D_H
#define MOROBOT_2D_H

#include "morobot.h"

class morobot_2d:public morobotClass {
	public:
		/**
		 *  \brief Constructor of morobot_2d class
		 *  \details The value in brakets defines that the robot consists of two smartservos
		 */
		morobot_2d() : morobotClass(2){memcpy(_robotJointLimits, _jointLimits, 3*2*sizeof(long)); memcpy(_robotAxisLimits, _axisLimits, 3*2*sizeof(uint8_t));};
		
		/**
		 *  \brief Set the position of the TCP (tool center point) with respect to the center of the flange of the last robot axis.
		 *  		This information is necessary to calculate the inverse kinematics correctly.
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
		
		const String type = "morobot_2d";		//!< Type of morobot
		
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
		long _jointLimits[3][2] = {{-110, 135}, {-72, 24}, {0, 0}};		//!< Limits for all joints
		uint8_t _axisLimits[3][2] = {{-20, 280}, {74.24, 74.24}, {110, 240}};	//!< Limits of x, y, z axis
		
		float x_def_offset = 40.96;			//!< Default offset from side of robot to first motor
		float y_def_offset = 74.24;			//!< Default offset from side of robot to center of eef
		float z_def_offset_bottom = 22;		//!< Default offset from side of robot to center of motors
		float z_def_offset_top = 16.9;		//!< Default offset from intersection of the second links and tcp
		
		float L1 = 100;			//!< Length of first link
		float L2 = 180;			//!< Lenth of second link
		float L3 = 0;			//!< Distance between the links at the eef
		float L4 = 78.08;		//!< Distance between the two motors
};

#endif