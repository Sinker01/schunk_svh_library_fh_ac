//
// Created by sven on 25.05.24.
//
#include "schunk_svh_library/Finger.h"
#include "schunk_svh_library/FingerManagerWrapper.h"

JNIEXPORT void JNICALL Java_FiveFingerManager_initFiveFingerManager(JNIEnv *env, jobject obj) {
  // Call the C function
  initFiveFingerManager();
}

JNIEXPORT jchar JNICALL Java_FiveFingerManager_setPositionTarget(JNIEnv *env, jobject obj, jint finger, jdouble position) {
  return setPositionTarget(finger, position);
}

JNIEXPORT jchar JNICALL Java_FiveFingerManager_setSpeed(JNIEnv *env, jobject obj, jint finger, jdouble speed) {
  return setSpeed(finger, speed);
}

JNIEXPORT jchar JNICALL Java_FiveFingerManager_setMaxNewton(JNIEnv *env, jobject obj, jint finger, jdouble newton) {
  return setMaxNewton(finger, newton);
}

JNIEXPORT jshort JNICALL Java_FiveFingerManager_getmA(JNIEnv *env, jobject obj, jint finger) {
  return getmA(finger);
}

JNIEXPORT jdouble JNICALL Java_FiveFingerManager_getNewton(JNIEnv *env, jobject obj, jint finger) {
  return getNewton(finger);
}

JNIEXPORT jdouble JNICALL Java_FiveFingerManager_getPosition(JNIEnv *env, jobject obj, jint finger) {
  return getPosition(finger);
}