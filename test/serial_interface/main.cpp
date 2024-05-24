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

void sleepAndGetData(const int finger, ostream& outfile, const clock_t time)
{
  clock_t start_time = clock();
  clock_t current_time;
  do
  {
    current_time = clock() - start_time;
    outfile << to_string(clock()-start_time) + ";" + to_string(get_mA(finger)) + ";" + to_string(getPosition(finger)) + "\n";
  } while (current_time <= time);
}

void getData(const unsigned count)
{
  int finger = 3;
  initFiveFingerManager();
  cout << "start";
  setSpeed(finger, 0.2);
  ofstream outfile{"outof" + to_string(finger)};
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

int main()
{
  getData();
  return 0;
}