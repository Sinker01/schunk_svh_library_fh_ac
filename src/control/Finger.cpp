#include <jni.h>
#include <iostream>
#include "schunk_svh_library/fingermanager_Finger.h"
#include "schunk_svh_library/FingerManagerWrapper.h"

extern "C" {

JNIEXPORT jshort JNICALL Java_fingermanager_Finger_getmA(JNIEnv* env, jclass cls, jint finger) {
    return getmA(finger);
}

JNIEXPORT void JNICALL Java_fingermanager_Finger_initFiveFingerManager(JNIEnv *env, jclass cls) {
  initFiveFingerManager();
}

JNIEXPORT jbyte JNICALL Java_fingermanager_Finger_setPositionTarget(JNIEnv *env, jclass cls, jint finger, jdouble position) {
  return setPositionTarget(finger, position);
}

JNIEXPORT jbyte JNICALL Java_fingermanager_Finger_setSpeed(JNIEnv *env, jclass cls, jint finger, jdouble speed) {
  return setSpeed(finger, speed);
}

JNIEXPORT jbyte JNICALL Java_fingermanager_Finger_setMaxNewton(JNIEnv *env, jclass cls, jint finger, jdouble newton) {
  return setMaxNewton(finger, newton);
}

JNIEXPORT jbyte JNICALL Java_fingermanager_Finger_setMaxmA(JNIEnv *env, jclass cls, jint finger, jdouble mA) {
  return setMaxmA(finger, mA);
}

JNIEXPORT jdouble JNICALL Java_fingermanager_Finger_getNewton(JNIEnv *env, jclass cls, jint finger) {
  return getNewton(finger);
}

JNIEXPORT jdouble JNICALL Java_fingermanager_Finger_getPosition(JNIEnv *env, jclass cls, jint finger) {
  return getPosition(finger);
}

}
