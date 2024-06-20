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

void sleepAndGetData(const int finger, ostream& outfile, const std::chrono::milliseconds time)
{
  auto start_time = std::chrono::system_clock::now();
  std::chrono::nanoseconds current_time;
  do
  {
    current_time = std::chrono::system_clock::now() - start_time;
    auto current_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time).count();
    outfile << to_string(current_time_ms) + ";"
                 + to_string(getmA(finger))
                 + ";" + to_string(getNewton(finger))
                 + ";" + to_string(getPosition(finger))
                 + "\n";
  } while (current_time <= time);
}

void sleepAndGetData_frequent(const int finger, ostream& file, const std::chrono::milliseconds time, std::chrono::milliseconds fr, unsigned &count)
{
  double cur, pos, newton;
  for(int j = 0; j < 100; j++)
  {
    count++;
    cout << count << ';' << cur << ';'<< newton << ";" << pos << endl;
    file << count << ';' << cur << ';'<< newton << ";" << pos << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    cur = getmA(finger);
    pos = getPosition(finger);
    newton = getNewton(finger);
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
  ofstream file("out.csv");
  //auto &file = cout;
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