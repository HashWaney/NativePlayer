package com.android.media.pthreaddemo;

public class ThreadHandler {

    private Callback callback;

    static {
        System.loadLibrary("native-lib");

    }


    public native void normalCreateThread();


    public native void mutexThread();


    public native void callbackFromCplus();

    public void onCallback(int type ,String msg) {
        if (callback != null) {
            callback.onCallback(type,msg);
        }

    }

    public void setOnCallback(Callback callback) {
        this.callback = callback;
    }


    public interface Callback {
        void onCallback(int type ,String msg);
    }
}
