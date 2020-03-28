package com.android.media.pthreaddemo;

public class ThreadHandler {

    static {
        System.loadLibrary("native-lib");

    }


    public native void normalCreateThread();


    public native void mutexThread();


    public native void callbackFromCplus();
}
