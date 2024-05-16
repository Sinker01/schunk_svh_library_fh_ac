//
// Created by sven on 25.04.24.
//
#include "schunk_svh_library/FingerManagerWrapper.h"
#include <iostream>
#include <schunk_svh_library/control/SVHController.h>
//auto tst = driver_svh::SVHChannel::
using namespace std;

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

inline void sleep()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
}

inline void mainPosition()
{
  for(int j = 0; j < 10; j++)
  {
    setFinger(j, 0.);
  }
  setFinger(8, 0.3);
  sleep();
}

int main()
{
  // initFiveFingerManager();
  // setFinger(8, 0.3);
  // for(int i = 0; i < 100; i++)
  // {
  //   cout << i << endl;
  //   setFinger(5, i*0.01);
  //   apply();
  //   sleep();
  //   mainPosition();
  //   sleep();
  // }
  initFiveFingerManager();
  mainPosition();
  sleep();
  for(int i = 0; i < CHANNELS; i++) setFinger(i, 0.);
  apply();
  sleep();
  for(int i = 0; i < CHANNELS; i++) setFinger(i, 1.);
  apply();
  sleep();
}