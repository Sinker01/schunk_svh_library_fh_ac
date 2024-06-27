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



void sleepAndGetData(const int finger, ostream& outfile, const size_t time)
{
  auto current_time = 0;
  for(int i = 0; i < 3000; i++) {
    this_thread::sleep_for(10ms);
    outfile << to_string(current_time) + ";" + to_string(getmA(finger)) + ";" + to_string(getPosition(finger)) + "\n";
  }
}

void getData(const unsigned count)
{
  int finger = 3;
  initFiveFingerManager();
  cout << "start";
  setSpeed(finger, 0.2);
  //ofstream outfile{"outof" + to_string(finger)};
  auto &outfile = cout;
  outfile << "time[µs];mA;position" << endl;
  for(unsigned i = 0; i < count; i++)
  {
    constexpr clock_t ONE_SEC = 1000000;
    setPositionTarget(finger, 1);
    sleepAndGetData(finger, outfile, ONE_SEC);
    setPositionTarget(finger, 0);
    sleepAndGetData(finger, outfile, ONE_SEC);
  }
}

void testNewton() {
  int finger = 6;
  initFiveFingerManager();
  //ofstream file("out.csv");
  auto &file = cout;
  file << "start\n\n";
  setPositionTarget(finger, 0);
  sleep(3000ms);

  cout << "time[µs];mA;Newton;position" << endl;
  for(int i = 0; i < 10; i++) {
    setMaxNewton(finger, -10);
    setPositionTarget(finger, 1);
    sleepAndGetData(3, file, ONE_SEC * 2);
    setPositionTarget(finger, 0);
    sleepAndGetData(3, file, ONE_SEC);
  }
}

void testmA() {
  int finger = 6;
  initFiveFingerManager();
  //ofstream file("out.csv");
  auto &file = cout;
  cout << "start\n\n";
  setPositionTarget(finger, 0);
  sleep(3000ms);

  cout << "time[µs];mA;Newton;position" << endl;
  unsigned count = 0;
  for(int i = 0; i < 10; i++) {
    //setMaxmA(finger, (10-i)/10. * 1e6);
    setPositionTarget(finger, 1);
    sleepAndGetData_frequent(3, file, 300ms, 1ms, count);
    setPositionTarget(finger, 0);
    sleepAndGetData_frequent(3, file, 200ms, 1ms, count);
  }
}

int main()
{
  testmA();
  //sleep(5000ms);
  //testmA();
  return 0;
}

int main()
{
  getData(2);
  return 0;
}