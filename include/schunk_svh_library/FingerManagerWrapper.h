//
// Created by sven on 16.05.24.
//
#pragma once

#ifndef FINGERMANAGERWRAPPER_H
#  define FINGERMANAGERWRAPPER_H
#  include "control/SVHController.h"

#endif //FINGERMANAGERWRAPPER_H

#define CHANNELS 9

void initFiveFingerManager();

void setFinger(int finger, double position);
void setSpeed(int finger, double speed);

double get_mA(int finger);
double getPosition(int finger);

inline void apply() {}
void getData();

extern const double RANGES[9][2];

inline auto castFinger(int finger)
{
  return static_cast<driver_svh::SVHChannel>(finger);
}

