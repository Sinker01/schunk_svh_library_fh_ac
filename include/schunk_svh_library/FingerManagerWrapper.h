//
// Created by sven on 16.05.24.
//
#pragma once

#ifndef FINGERMANAGERWRAPPER_H
#  define FINGERMANAGERWRAPPER_H
#  include "control/SVHController.h"

#endif //FINGERMANAGERWRAPPER_H

#define CHANNELS 9

/**
 * Initialises the five finger manager. This method should be called first.
 */
void initFiveFingerManager();

/**
 * An method to set the position of a finger.
 * The code continues after calling thos method, so you program shpuld propably wait.
 * @param finger The finger which should be accessed
 * @param position The position between 0 and 1
 */
char setPositionTarget(int finger, double position);
/**
 *  the speed to the finger
 *   Set 1 for the maximun speed. Otherwise, the speed will be determined by multiplying the maximun
 * speed with the given speed
 * @param finger
 * @param speed
 */
char setSpeed(int finger, double speed);
char setMaxNewton(int finger, double newton);
char setMaxmA(int finger, uint16_t mA);

int16_t getmA(int finger);
double getNewton(int finger);
double getPosition(int finger);
