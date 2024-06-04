//
// Created by sven on 25.05.24.
//
#include "schunk_svh_library/fingermanager_Finger.h"
#include "schunk_svh_library/FingerManagerWrapper.h"

JNIEXPORT void JNICALL Java_fingermanager_Finger_initFiveFingerManager(JNIEnv *env, jobject obj) {
  // Call the C function
  initFiveFingerManager();
}

JNIEXPORT jbyte JNICALL Java_fingermanager_Finger_setPositionTarget(JNIEnv *env, jobject obj, jint finger, jdouble position) {
  return setPositionTarget(finger, position);
}

JNIEXPORT jbyte JNICALL Java_fingermanager_Finger_setSpeed(JNIEnv *env, jobject obj, jint finger, jdouble speed) {
  return setSpeed(finger, speed);
}

JNIEXPORT jbyte JNICALL Java_fingermanager_Finger_setMaxNewton(JNIEnv *env, jobject obj, jint finger, jdouble newton) {
  return setMaxNewton(finger, newton);
}

JNIEXPORT jshort JNICALL Java_fingermanager_Finger_getmA(JNIEnv *env, jobject obj, jint finger) {
  return getmA(finger);
}

JNIEXPORT jdouble JNICALL Java_fingermanager_Finger_getNewton(JNIEnv *env, jobject obj, jint finger) {
  return getNewton(finger);
}

JNIEXPORT jdouble JNICALL Java_fingermanager_Finger_getPosition(JNIEnv *env, jobject obj, jint finger) {
  return getPosition(finger);
}