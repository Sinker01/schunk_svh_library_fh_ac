//
// Created by sven on 24.05.24.
//

#include "schunk_svh_library/control/SVHFingerManager.h"
#include <fstream>
#include <schunk_svh_library/control/SVHController.h>
#include <thread>

#include "schunk_svh_library/FingerManagerWrapper.h"

using namespace std;

void sleep(std::chrono::milliseconds time = 2000ms)
{
  this_thread::sleep_for(time);
}

void testAllFingerSpread()
{
  initFiveFingerManager();
  for(int i = 0; i < CHANNELS; i++)
  {
    cout << i << endl;
    setPositionTarget(i, 1);
    sleep();
    setPositionTarget(i, 0);
    sleep();
  }
}

void testSpeed()
{
  initFiveFingerManager();
  cout << "start";
  for(int i = 0; i < 10; i++)
  {
    constexpr int FINGER = 4;
    setSpeed(FINGER, 1.-(i/10.));
    setPositionTarget(FINGER, 1);
    sleep();
    setPositionTarget(FINGER, 0);
    sleep();
  }
}

void mainPosition()
{
  for(int j = 0; j < 10; j++)
  {
    setPositionTarget(j, 0.);
  }
  setPositionTarget(8, 0.3);
  sleep(1000ms);
}

size_t anz = 0;

void sleepAndGetData(const int finger, vector<ostream*>& outfile)
{
  for(int i = 0; i < 1000; i++) {
    this_thread::sleep_for(10ms);
    string out = to_string(anz) + ";" 
    + to_string(getmA(finger)) + ";" 
    + to_string(getPosition(finger)) + ";" 
    + to_string(getNewton(finger)) + "\n";
    for(auto &f: outfile) *f << out;
    anz++;
  }
}

void testmA()
{
  int finger = 5;
  initFiveFingerManager();
  cout << "start";
  setSpeed(finger, 0.2);
  ofstream outfile{"outof" + to_string(finger)};
  vector<ostream*> out {&cout, &outfile};
  outfile << "time[µs];mA;position" << endl;
  for(unsigned i = 0; i < 10; i++)
  {
    setMaxmA(finger, (10 - i) * 36);
    setPositionTarget(finger, 1);
    sleepAndGetData(finger, out);
    setPositionTarget(finger, 0);
    sleepAndGetData(finger, out);
  }
}

int main()
{
  testmA();
  return 0;
}