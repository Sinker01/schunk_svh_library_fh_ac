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
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
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

void testAllFingerSpread()
{
  initFiveFingerManager();
  for(int i = 0; i < CHANNELS; i++)
  {
    cout << i << endl;
    setFinger(i, 1);
    apply();
    sleep();
    setFinger(i, 0);
    apply();
    sleep();
  }
}

void testSpeed()
{
  initFiveFingerManager();
  cout << "start";
  int finger = 4;
  for(int i = 0; i < 10; i++)
  {
    setSpeed(finger, 1.-(i/10.));
    setFinger(finger, 1);
    apply();
    sleep();
    setFinger(finger, 0);
    apply();
    sleep();
  }
}

int main()
{
  getData();
  return 0;
}