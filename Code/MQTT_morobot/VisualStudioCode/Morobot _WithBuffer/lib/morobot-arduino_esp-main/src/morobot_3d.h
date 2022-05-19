/**
 *  \class 	morobot_3d
 *  \brief 	morobot child class for morobot-3d for microcontrollers such as Arduino or ESP32
 *  @file 	morobot_3d.h
 *  @author	Johannes Rauer FHTW
 *  @date	2020/11/27
 *  \par Method List:
 *  	public:
 *  		morobot_3d() : morobotClass(3){};
			virtual void setTCPoffset(float xOffset, float yOffset, float zOffset);
			virtual bool checkIfAngleValid(uint8_t servoId, float angle);
		protected:
			virtual bool calculateAngles(float x, float y, float z);
			uint8_t calculateAngleYZ(float x, float y, float z, float &theta);
			virtual void updateCurrentXYZ();
 */

#ifndef MOROBOT_3D_H
#define MOROBOT_3D_H

#include "morobot.h"

class morobot_3d:public morobotClass {
	public:
		/**
		 *  \brief Constructor of morobot_3d class
		 *  \details The value in brakets defines that the robot consists of three smartservos
		 */
		morobot_3d() : morobotClass(3){memcpy(_robotJointLimits, _jointLimits, 3*2*sizeof(long)); memcpy(_robotAxisLimits, _axisLimits, 3*2*sizeof(uint8_t));};
		
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
		
		const String type = "morobot_3d";		//!< Type of morobot
		
	protected:
		/**
		 *  \brief Uses given coordinates to calculate the motor angles to reach this position (Solve inverse kinematics).
		 *			This function does only calculate the angles of the motors and stores them internally.
		 *  		Use moveToPose(x,y,z) to actually move the robot.
		 *  \param [in] x Desired x-position of TCP
		 *  \param [in] y Desired y-position of TCP
		 *  \param [in] z Desired z-position of TCP
		 *  \return Returns true if the position is reachable; false if it is not.
		 */
		virtual bool calculateAngles(float x, float y, float z);
		
		/**
		 *  \brief Helper function for inverse kinematicts that calculates the motor angle in the plane.
		 *  \param [in] x Desired x-position of TCP
		 *  \param [in] y Desired y-position of TCP
		 *  \param [in] z Desired z-position of TCP
		 *  \param [in] theta Angle of motor to calculate
		 *  \return Returns 0 if calculation succeeds, -1 otherwise
		 */
		uint8_t calculateAngleYZ(float x, float y, float z, float &theta);
		
		/**
		 *  \brief Re-calculates the internally stored robot TCP position (Solves forward kinematics).
		 *  		This function does calculate and store the TCP position depending on the current motor angles.
		 */
		virtual void updateTCPpose(bool output = false);

	private:
		float _tcpOffset[3];	//!< Position of the TCP (tool center point) with respect to the center of the flange of the last robot axis
		long _jointLimits[3][2] = {{0, 85}, {0, 85}, {0, 85}};		//!< Limits for all joints
		uint8_t _axisLimits[3][2] = {{-80, 80}, {-80, 80}, {112, 235}};	//!< Limits of x, y, z axis

		float z_def_offset_top = 24.0;			//!< Default offset from top side of robot to motor axes
		float z_def_offset_bottom = 10.0;		//!< Default offset from bottom side of end effector triangle to rotation axis of last joint

		float e = 69.28;	//!< Length of side of end effector triangle
		float f = 178.78;	//!< Length of side of base triangle
		float re = 149.95;	//!< Length of parallelogram link
		float rf = 96.00;	//!< Length of upper link
		
		// trigonometric constants
		const float sqrt3 = sqrt(3.0);
		const float sin120 = sqrt3/2.0;   
		const float cos120 = -0.5;        
		const float tan60 = sqrt3;
		const float sin30 = 0.5;
		const float tan30 = 1/sqrt3;
};

#endif