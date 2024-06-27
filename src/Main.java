import fingermanager.Finger;

public class Main {
    public static void main(String[] args) {
        // Initialize the finger manager
        Finger.initFiveFingerManager();

        // Running the provided test functions
        ///testAllFingerSpread();
        //testSpeed();
        //mainPosition();
        //getData(5);
        helloWorld();
        //helloWorld();
    }

    private static void helloWorld(){
        for (Finger finger : Finger.values()){
            finger.setPositionTarget(1.0);
        }
        sleep(2000);
        Finger.MIDDLE_DISTAL.setPositionTarget(0.0);
        Finger.MIDDLE_PROXIMAL.setPositionTarget(0.0);
        sleep(2000);

    }

    // Wrapper methods to call the corresponding native functions
    private static void testAllFingerSpread() {
        for (Finger finger : Finger.values()) {
            System.out.println("Testing finger: " + finger.name());
            finger.setPositionTarget(1.0);
            sleep(2000);
            finger.setPositionTarget(0.0);
            sleep(2000);
        }
    }

    private static void testSpeed() {
        Finger finger = Finger.MIDDLE_DISTAL;
        for (int i = 0; i < 10; i++) {
            double speed = 1.0 - (i / 10.0);
            finger.setSpeed(speed);
            finger.setPositionTarget(1.0);
            sleep(2000);
            finger.setPositionTarget(0.0);
            sleep(2000);
        }
    }

    private static void mainPosition() {
        for (Finger finger : Finger.values()) {
            finger.setPositionTarget(0.0);
        }
        Finger.SPREAD.setPositionTarget(0.3);
        sleep(1000);
    }

    private static void getData(int count) {
        Finger finger = Finger.INDEX_PROXIMAL;
        for (int i = 0; i < count; i++) {
            finger.setPositionTarget(1.0);
            sleepAndPrintData(finger, 1000000);
            finger.setPositionTarget(0.0);
            sleepAndPrintData(finger, 1000000);
        }
    }

    private static void testNewton() {
        Finger finger = Finger.RING;
        for (int i = 0; i < 10; i++) {
            finger.setMaxNewton((10 - i) * 0.5);
            finger.setPositionTarget(1.0);
            sleepAndPrintData(finger, 1000000);
            finger.setPositionTarget(0.0);
            sleepAndPrintData(finger, 1000000);
        }
    }

    // Helper method to sleep for the specified milliseconds
    private static void sleep(int milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    // Helper method to sleep and print data
    private static void sleepAndPrintData(Finger finger, long timeMicros) {
        long startTime = System.nanoTime();
        long currentTime;
        do {
            currentTime = System.nanoTime() - startTime;
            System.out.println("Time: " + currentTime / 1000 + "µs; mA: " + finger.getmA() + "; Newton: " + finger.getNewton() + "; Position: " + finger.getPosition());
        } while (currentTime <= timeMicros * 1000);
    }
}
