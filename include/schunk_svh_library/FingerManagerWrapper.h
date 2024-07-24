//
// Created by sven on 16.05.24.
//
#pragma once

#ifndef FINGERMANAGERWRAPPER_H
#  define FINGERMANAGERWRAPPER_H
#  include "control/SVHController.h"

#endif //FINGERMANAGERWRAPPER_H
#include "schunk_svh_library/ImportExport.h"

#define CHANNELS 9

constexpr auto PORT = 
#ifdef _SYSTEM_WIN32_
"COM3"
#else
"/dev/ttyUSB0"
#endif
;


/**
 * Initialises the five finger manager. This method should be called first.
 */
void DRIVER_SVH_IMPORT_EXPORT initFiveFingerManager(const char* const port = PORT);

/**
 * An method to set the position of a finger.
 * The code continues after calling thos method, so you program shpuld propably wait.
 * @param finger The finger which should be accessed
 * @param position The position between 0 and 1
 */
char DRIVER_SVH_IMPORT_EXPORT setPositionTarget(int finger, double position);
/**
 *  the speed to the finger
 *   Set 1 for the maximun speed. Otherwise, the speed will be determined by multiplying the maximun
 * speed with the given speed
 * @param finger
 * @param speed
 */
char DRIVER_SVH_IMPORT_EXPORT setSpeed(int finger, double speed);
char DRIVER_SVH_IMPORT_EXPORT setMaxNewton(int finger, double newton);
char DRIVER_SVH_IMPORT_EXPORT setMaxmA(int finger, uint16_t mA);

int16_t DRIVER_SVH_IMPORT_EXPORT getmA(int finger);
double DRIVER_SVH_IMPORT_EXPORT getNewton(int finger);
double DRIVER_SVH_IMPORT_EXPORT getPosition(int finger);
