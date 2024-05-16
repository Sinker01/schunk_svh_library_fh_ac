//
// Created by sven on 16.05.24.
//

#include "schunk_svh_library/FingerManagerWrapper.h"

//
// Created by sven on 25.04.24.
//

#include "schunk_svh_library/control/SVHFingerManager.h"
#include <iostream>
#include <schunk_svh_library/control/SVHController.h>
#include <vector>

/*
0	thumb_flexion
1	th
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

double RANGES[9][2];

void initFiveFingerManager()
{
  if (!g_m_svh.connect())
  {
    cout << "test";
    throw "Could not be connected";
  }
  auto firmware = g_m_svh.getFirmwareInfo();
  auto version =
    std::to_string(firmware.version_major) + "." + std::to_string(firmware.version_minor) + ".";
  cout << version;

  SVHCurrentSettings current_settings[9]{
    {-605.0, 605.0, 0.405, 10.0e-6, -0.125, 0.125, 0., 1984, -248.0, 248.0},
    {-605.0, 605.0, 0.405, 10.0e-6, -0.125, 0.125, 0., 1984, -248.0, 248.0},
    {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
    {-688.0, 688.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
    {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
    {-350.0, 350.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
    {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
    {-360.0, 360.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0},
    {-500.0, 500.0, 0.405, 10.0e-6, -0.125, 0.125, 0, 1984, -248.0, 248.0}};

  SVHPositionSettings position_settings[9]{
    {-1.0e+6, 1.0e+6, 65.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.6, 0.1, 2000.0},
    {-1.0e+6, 1.0e+6, 50.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.6, 0.1, 2000.0},
    {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.4, 0.36, 2400},
    {-1.0e+6, 1.0e+6, 40.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.2, 0.17, 1600},
    {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.4, 0.36, 2400},
    {-1.0e+6, 1.0e+6, 40.0e+3, 1.00, 1.0e-3, -2000.0, 2000.0, 0.2, 0.17, 1600},
    {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.6, 0.0, 2500},
    {-1.0e+6, 1.0e+6, 45.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 0.6, 0.0, 2500},
    {-1.0e+6, 1.0e+6, 25.0e+3, 1.00, 1.0e-3, -1000.0, 1000.0, 1.0, 0.36, 4000}};

  SVHHomeSettings home_settings[9]{{+1, -175.0e+3, -5.0e+3, -15.0e+3, 0.97, 0.75},
                                   {+1, -105.0e+3, -5.0e+3, -15.0e+3, 0.99, 0.75},
                                   {+1, -47.0e+3, -2.0e+3, -8.0e+3, 1.33, 0.75},
                                   {-1, 2.0e+3, 42.0e+3, -8.0e+3, 0.8, 0.75},
                                   {+1, -47.0e+3, -2.0e+3, -8.0e+3, 1.33, 0.75},
                                   {-1, 2.0e+3, 42.0e+3, -8.0e+3, 0.8, 0.75},
                                   {+1, -47.0e+3, -2.0e+3, -8.0e+3, 0.98, 0.75},
                                   {+1, -47.0e+3, -2.0e+3, -8.0e+3, 0.98, 0.75},
                                   {+1, -57.0e+3, -2.0e+3, -25.0e+3, 0.58, 0.4}};

  for (size_t i = 0; i < driver_svh::SVH_DIMENSION; ++i)
  {
    g_m_svh.setCurrentSettings(static_cast<driver_svh::SVHChannel>(i), current_settings[i]);

    g_m_svh.setPositionSettings(static_cast<driver_svh::SVHChannel>(i), position_settings[i]);

    g_m_svh.setHomeSettings(static_cast<driver_svh::SVHChannel>(i), home_settings[i]);
  }

  auto m_initialized = g_m_svh.resetChannel(driver_svh::SVHChannel::SVH_ALL);
  if (!m_initialized)
  {
    throw "Could not initialize the Schunk SVH";
  }

  for(int i = 0; i < CHANNELS; i++)
  {
    RANGES[i][0] = 0.;
    RANGES[i][1] = g_m_svh.convertTicks2Rad(static_cast<SVHChannel>(i), g_m_svh.m_position_max[i]);
  }

  RANGES[8][0] = 0.3;
  for(int i = 0; i < CHANNELS; i++)
  {
    g_positions[i] = RANGES[i][0];
  }

  g_m_svh.setAllTargetPositions(g_positions);
}
void setFinger(int finger, double position) {
  g_positions[finger] = RANGES[finger][0] + ((RANGES[finger][1] - RANGES[finger][0]) * position);
}

void apply()
{
  g_m_svh.setAllTargetPositions(g_positions);
}
