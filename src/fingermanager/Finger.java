package fingermanager;

import java.io.File;

public enum Finger {

    THUMB_FLEXION(0),
    THUMB_DISTAL(1),
    INDEX_DISTAL(2),
    INDEX_PROXIMAL(3),
    MIDDLE_DISTAL(4),
    MIDDLE_PROXIMAL(5),
    RING(6),
    PINKY(7),
    SPREAD(8);

    private final int index;

    Finger(int index) {
        this.index = index;
    }

    public boolean setPositionTarget(double target) {
        return !(setPositionTarget(index, target) == 0);
    }

    public boolean setSpeed(double speed) {
        return !(setSpeed(index, speed) == 0);
    }

    public boolean setMaxNewton(double maxNewton) {
        return !(setMaxNewton(index, maxNewton) == 0);
    }

    public short getmA() {
        return getmA(0); // Example usage
    }

    public double getNewton() {
        return getNewton(index);
    }

    public double getPosition() {
        return getPosition(index);
    }



   // public static void initFingerManager(){initFiveFingerManager();}

//    static {
//        File lib = new File("c_project/cmake-build-debug/" + System.mapLibraryName("svh_java")); //Pfad in den cmake_build_debug Ordner, wo nach dem kompilieren die .so Dateien liegen
//        System.out.println("FINGER LIB PATH: " + lib.getAbsolutePath());
//        System.load(lib.getAbsolutePath());
//        initFiveFingerManager();
//    }

    static {
        System.loadLibrary("svh_java");
        System.out.println("svh_java library loaded.");
        //initFiveFingerManager();
    }

    public static native void initFiveFingerManager();

    private static native byte setPositionTarget(int finger, double position);

    private static native byte setSpeed(int finger, double speed);

    private static native byte setMaxNewton(int finger, double newton);

    private static native short getmA(int finger);

    private static native double getNewton(int finger);

    private static native double getPosition(int finger);
}
