/**
 *  \brief 	Checks for which board is compiled and adds corresponding dabble-library
 *  @file 	dabble_include.h
 *  @author	Johannes Rauer FHTW
 *  @date	2021/03/01
 */
#if defined(__AVR__)
	#include <Dabble.h>				// Include Dabble library for AVR-based controllers (Arduino) if no ESP32 is used
	#define DABBLE_PARAM 9600		// Set transmission speed
#elif defined(ESP32)
	#include <DabbleESP32.h>		// Include Dabble library for ESP32 board
	#define DABBLE_PARAM "MyEsp32" 	// Set bluetooth name
#else
	#error "Selected board not compatible with Dabble-library!"	// If no dabble library for board is found print error message
#endif