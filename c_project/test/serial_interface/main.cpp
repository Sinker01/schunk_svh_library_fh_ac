//
// Created by sven on 24.05.24.
//

#include "schunk_svh_library/control/SVHFingerManager.h"
#include <fstream>
#include <schunk_svh_library/control/SVHController.h>
#include <thread>

#include "schunk_svh_library/FingerManagerWrapper.h"

using namespace std;

constexpr auto ONE_SEC = 1000ms;

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

size_t anz;

void sleepAndGetData(const int finger, ostream& outfile)
{
  for(int i = 0; i < 300; i++) {
    this_thread::sleep_for(10ms);
    outfile << to_string(anz) + ";" + to_string(getmA(finger)) + ";" + to_string(getPosition(finger)) + "\n";
    anz++;
  }
}

void testNewton() {
  int finger = 6;
  initFiveFingerManager();
  ofstream file("out.csv");
  //auto &file = cout;
  cout << "start\n\n";
  setPositionTarget(finger, 0);
  sleep(100ms);

  cout << "time[µs];mA;position;Newton" << endl;
  for(int i = 0; i < 10; i++) {
    setMaxNewton(finger, (10-i) * 0.5);
    setPositionTarget(finger, 1);
    sleepAndGetData(3, file);
    setPositionTarget(finger, 0);
    sleepAndGetData(3, file);
  }
}

void testmA() {
  int finger = 6;
  initFiveFingerManager();
  //ofstream file("out.csv");
  auto &file = cout;
  cout << "start\n\n";
  setPositionTarget(finger, 0);
  sleep(100ms);

  cout << "time[µs];mA;position;Newton" << endl;
  for(int i = 0; i < 10; i++) {
    setMaxmA(finger, (10-i)*36);
    setPositionTarget(finger, 1);
    sleepAndGetData(3, file);
    setPositionTarget(finger, 0);
    sleepAndGetData(3, file);
  }
}

int main()
{
  anz = 0;
  testmA();
  //sleep(5000ms);
  //testmA();
  return 0;
}