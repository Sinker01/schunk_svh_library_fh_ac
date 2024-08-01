//
// Created by sven on 16.05.24.
//

#include "schunk_svh_library/FingerManagerWrapper.h"
#include <cmath>
#include "schunk_svh_library/control/SVHFingerManager.h"
#include <schunk_svh_library/control/SVHController.h>
#include <vector>
#include <iostream>

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

//an instance of the SVHFingerManager, which controls the hand
SVHFingerManager g_m_svh;

//The target positions of the Fingers
vector<double> g_positions(9);
/*
 * The SVHPositionSettings, SVHCurrentSettings and SVHHomeSettings were running configurations from the ros2 driver.
 * A few values from POSITION_SETTINGS and CURRENT_SETTINGS gets changed in the program, to change speed and the maximal force o the program
 * TODO find out, what these values do, and optimize them
 */
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

/*
 * Define the minimal finger spread for each finger.
 * For finger_spread, the fingers would infer
 */
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

//Variables for the actual position- and current_settings, which will be
SVHPositionSettings position_settings[9];
SVHCurrentSettings current_settings[9];

/**
 * Initialises the Finger manager and all previous called variables
 * @param port The used port to connect with
 */
void initFiveFingerManager(const char* const port)
{

  for(int i = 0; i < CHANNELS; i++)
  {
    position_settings[i] = POSITION_SETTINGS[i];
    current_settings[i] = CURRENT_SETTINGS[i];
  }

  //Connect to the usb driver
  if (!g_m_svh.connect(port))
  {
    throw "Could not be connected";
  }

  //get and print firmware version
  auto firmware = g_m_svh.getFirmwareInfo();
  auto version =
    std::to_string(firmware.version_major) + "." + std::to_string(firmware.version_minor) + ".";
  cout << version;

  //Apply the initial settings
  for (size_t i = 0; i < driver_svh::SVH_DIMENSION; ++i)
  {
    g_m_svh.setCurrentSettings(static_cast<driver_svh::SVHChannel>(i), current_settings[i]);
    g_m_svh.setPositionSettings(static_cast<driver_svh::SVHChannel>(i), position_settings[i]);
    g_m_svh.setHomeSettings(static_cast<driver_svh::SVHChannel>(i), HOME_SETTINGS[i]);
  }

  //reset all fingers. This will take a while...
  auto m_initialized = g_m_svh.resetChannel(driver_svh::SVHChannel::SVH_ALL);
  if (!m_initialized)
  {
    throw "Could not initialize the Schunk SVH";
  }

  //Apply the minimal finger spread
  for(int i = 0; i < CHANNELS; i++) g_positions[i] = MAX_RANGE_RAD[i];
  g_m_svh.setAllTargetPositions(g_positions);
}

/**
 * Helper method to cast a finger as int into an driver_svh::SVHChannel
 * @param finger the finger as int
 * @return the finger as driver_svh::SVHChannel
 */
inline constexpr auto castFinger(int finger)
{
  return static_cast<driver_svh::SVHChannel>(finger);
}

int16_t getmA(int finger)
{
  int16_t ret;
  if(!g_m_svh.getCurrent(castFinger(finger), ret)) return INT16_MAX;
  return ret;
}

/**
 * Gets the current Force by get the current Newton and convert it via the convertmAtoN method
 * @param finger
 * @return The force in Netwon
 */
double getNewton(int finger)
{
  g_m_svh.requestControllerFeedback(castFinger(finger));
  auto mA = getmA(finger);
  if(mA==INT16_MAX) return NAN;
  //TODO understand and optimize the convertmAtoN method
  return g_m_svh.convertmAtoN(castFinger(finger), mA);
}

/**
 * Gets the position
 * The return value gets modified so the minimal and maximal values are 0 and 1
 * @param finger
 * @return The position between 0 and 1
 */
double getPosition(int finger)
{
  double pos;
  if(!g_m_svh.getPosition(castFinger(finger), pos)) return NAN;
  return (pos - MAX_RANGE_RAD[finger]) / (HOME_SETTINGS[finger].range_rad - MAX_RANGE_RAD[finger]);
}

/**
 * Sets the position target
 * The input value gets modified, so the minimal and maximal input values are 0 and 1.
 * @param finger
 * @param position the target position between 0 and 1
 * @return 0, if the value was ok; -1, if the value was too low; 1, if the value was too high
 */
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

/**
 * Sets the position target
 * @param finger
 * @param speed the target speed between 0 and 1
 * @return 0, if the value was ok; -1, if the value was too low; 1, if the value was too high
 */
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

char setMaxmA(int finger, double mA)
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
  //TODO understand and optimize the convertNtomA Method
  uint16_t mA = g_m_svh.convertNtomA(castFinger(finger), newton);
  return setMaxmA(finger, mA);
}