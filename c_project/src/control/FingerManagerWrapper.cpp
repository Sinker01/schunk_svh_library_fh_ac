//
// Created by sven on 16.05.24.
//

#include "schunk_svh_library/FingerManagerWrapper.h"

//
// Created by sven on 25.04.24.
//

#include <cmath>
#include "schunk_svh_library/control/SVHFingerManager.h"
#include <fstream>
#include <iostream>
#include <schunk_svh_library/control/SVHController.h>
#include <vector>

/*
0	thumb_flexion
1	thumb_distal
2	Index_distal
3	index_proximal
4	middle_distal
5	middle_proximal
6	ring
7	pinky
8	spread
*/

using namespace driver_svh;

using namespace std;

SVHFingerManager g_m_svh;
vector<double> g_positions(9);

const SVHPositionSettings POSITION_SETTINGS[9]{
  {-1.0e+6, 1.0e+6, 65.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.6, 0.1, 2000.0},
  {-1.0e+6, 1.0e+6, 50.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.6, 0.1, 2000.0},
  {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.4, 0.36, 2400},
  {-1.0e+6, 1.0e+6, 40.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.2, 0.17, 1600},
  {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.4, 0.36, 2400},
  {-1.0e+6, 1.0e+6, 40.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.2, 0.17, 1600},
  {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.6, 0.0, 2500},
  {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.6, 0.0, 2500},
{-1.0e+6, 1.0e+6, 25.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 1.0, 0.36, 4000}};

const SVHCurrentSettings CURRENT_SETTINGS[9]{
  {-605.0, 605.0, 0.405, 10.0e-6, -0.125, 0.125, 0., 1984, -248.0, 248.0},
  {-605.0, 605.0, 0.405, 10.0e-6, -0.125, 0.125, 0., 1984, -248.0, 248.0},
  {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
  {-688.0, 688.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
  {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
  {-350.0, 350.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
  {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
  {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
  {-500.0, 500.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0}};

const SVHHomeSettings HOME_SETTINGS[9]{
  {+1, -175.0e+3, -5.0e+3, -15.0e+3, 0.97, 0.75},
  {+1, -105.0e+3, -5.0e+3, -15.0e+3, 0.99, 0.75},
  {+1, -47.0e+3, -2.0e+3, -8.0e+3, 1.33, 0.75},
  {-1, 2.0e+3, 42.0e+3, -8.0e+3, 0.8, 0.75},
  {+1, -47.0e+3, -2.0e+3, -8.0e+3, 1.33, 0.75},
  {-1, 2.0e+3, 42.0e+3, -8.0e+3, 0.8, 0.75},
  {+1, -47.0e+3, -2.0e+3, -8.0e+3, 0.98, 0.75},
  {+1, -47.0e+3, -2.0e+3, -8.0e+3, 0.98, 0.75},
  {+1, -57.0e+3, -2.0e+3, -25.0e+3, 0.58, 0.4}};

constexpr double MAX_RANGE_RAD[9] {
  0.,
  0.,
  0.,
  0.,
  0.,
  0.,
  0.,
  0.,
  0.3
};

SVHPositionSettings position_settings[9];
SVHCurrentSettings current_settings[9];

void initFiveFingerManager()
{
  for(int i = 0; i < CHANNELS; i++)
  {
    position_settings[i] = POSITION_SETTINGS[i];
    current_settings[i] = CURRENT_SETTINGS[i];
  }
  if (!g_m_svh.connect())
  {
    cout << "test";
    throw "Could not be connected";
  }
  auto firmware = g_m_svh.getFirmwareInfo();
  auto version =
    std::to_string(firmware.version_major) + "." + std::to_string(firmware.version_minor) + ".";
  cout << version;

  for (size_t i = 0; i < driver_svh::SVH_DIMENSION; ++i)
  {
    g_m_svh.setCurrentSettings(static_cast<driver_svh::SVHChannel>(i), current_settings[i]);
    g_m_svh.setCurrentSettings(static_cast<driver_svh::SVHChannel>(i), current_settings[i]);

    g_m_svh.setPositionSettings(static_cast<driver_svh::SVHChannel>(i), position_settings[i]);

    g_m_svh.setHomeSettings(static_cast<driver_svh::SVHChannel>(i), HOME_SETTINGS[i]);
  }

  auto m_initialized = g_m_svh.resetChannel(driver_svh::SVHChannel::SVH_ALL);
  if (!m_initialized)
  {
    throw "Could not initialize the Schunk SVH";
  }
  for(int i = 0; i < CHANNELS; i++) g_positions[i] = MAX_RANGE_RAD[i];
  g_m_svh.setAllTargetPositions(g_positions);
}


inline auto castFinger(int finger)
{
  return static_cast<driver_svh::SVHChannel>(finger);
}

int16_t getmA(int finger)
{
  int16_t ret;
  if(!g_m_svh.getCurrent(castFinger(finger), ret)) return INT16_MAX;
  return ret;
}

double getNewton(int finger)
{
  g_m_svh.requestControllerFeedback(castFinger(finger));
  auto mA = getmA(finger);
  if(mA==INT16_MAX) return NAN;
  return g_m_svh.convertmAtoN(castFinger(finger), mA);
}

double getPosition(int finger)
{
  double pos;
  if(!g_m_svh.getPosition(castFinger(finger), pos)) return NAN;
  return pos / (HOME_SETTINGS[finger].range_rad - MAX_RANGE_RAD[finger]) - MAX_RANGE_RAD[finger];
}

char setPositionTarget(int finger, double position) {
  char ret = 0;
  if(position>1)
  {
    position = 1;
    ret = 1;
  }
  else if(position<0)
  {
    position = 0;
    ret = -1;
  }
  g_positions[finger] = MAX_RANGE_RAD[finger] + ((HOME_SETTINGS[finger].range_rad - MAX_RANGE_RAD[finger]) * position);
  g_m_svh.setAllTargetPositions(g_positions);
  return ret;
}

char setSpeed(int finger, double speed)
{
  char ret = 0;
  if(speed>1)
  {
    speed = 1;
    ret = 1;
  }
  else if(speed<0)
  {
    speed = 0;
    ret = -1;
  }
  position_settings[finger].dt = static_cast<float>(POSITION_SETTINGS[finger].dt * speed);
  g_m_svh.setPositionSettings(static_cast<driver_svh::SVHChannel>(finger), position_settings[finger]);
  return ret;
}

char setMaxmA(int finger, uint16_t mA)
{
  if(mA<0) mA = -mA;
  char ret = mA > POSITION_SETTINGS[finger].wmx;
  if(ret) mA = POSITION_SETTINGS[finger].wmx;
  current_settings[finger].wmn = -mA;
  current_settings[finger].wmx = mA;
  g_m_svh.setCurrentSettings(castFinger(finger), current_settings[finger]);
  return ret;
}

char setMaxNewton(int finger, double newton)
{
  uint16_t mA = g_m_svh.convertNtomA(castFinger(finger), newton);
  return setMaxmA(finger, mA);
}