package com.haiseiko.skeaclock;

public class SkeaClockNative {
    static {
        System.loadLibrary("native-lib");
    }

    public static native float[] getTimeAngles();
}
