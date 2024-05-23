//
// Created by sven on 16.05.24.
//
#pragma once

#ifndef FINGERMANAGERWRAPPER_H
#define FINGERMANAGERWRAPPER_H

#endif //FINGERMANAGERWRAPPER_H

#define CHANNELS 9

void initFiveFingerManager();
void test();

void setFinger(int finger, double position);
void setSpeed(int finger, double speed);
inline void apply() {}

extern const double RANGES[9][2];