package com.android.media.nativeplayerlib;

public class TestJNI {

    static {
        System.loadLibrary("native-lib");
    }

    public native String stringFromJNI();
}
