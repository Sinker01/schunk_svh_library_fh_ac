//
// Created by sven on 16.05.24.
//
#pragma once

#include "control/SVHController.h"
#include "schunk_svh_library/ImportExport.h"

#define CHANNELS 9

/**
 * @var PORT definition of the used port, if not passed in the initFiveFingerManager method
 */
constexpr auto PORT = 
#ifdef _SYSTEM_WIN32_
"COM3"
#else
"/dev/ttyUSB0"
#endif
;


//The usage of this method are quite same as in the java documentation.
void DRIVER_SVH_IMPORT_EXPORT initFiveFingerManager(const char* const port = PORT);
char DRIVER_SVH_IMPORT_EXPORT setPositionTarget(int finger, double position);
char DRIVER_SVH_IMPORT_EXPORT setSpeed(int finger, double speed);
char DRIVER_SVH_IMPORT_EXPORT setMaxNewton(int finger, double newton);
char DRIVER_SVH_IMPORT_EXPORT setMaxmA(int finger, double mA);
int16_t DRIVER_SVH_IMPORT_EXPORT getmA(int finger);
double DRIVER_SVH_IMPORT_EXPORT getNewton(int finger);
double DRIVER_SVH_IMPORT_EXPORT getPosition(int finger);
