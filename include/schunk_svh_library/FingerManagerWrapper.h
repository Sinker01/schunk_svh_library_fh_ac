//
// Created by sven on 16.05.24.
//
#pragma once

#ifndef FINGERMANAGERWRAPPER_H
#define FINGERMANAGERWRAPPER_H

#endif //FINGERMANAGERWRAPPER_H

#define CHANNELS 9

void initFiveFingerManager();

void setFinger(int finger, double position);
void apply();

extern double RANGES[9][2];