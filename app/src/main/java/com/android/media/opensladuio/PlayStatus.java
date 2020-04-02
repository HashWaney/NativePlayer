package com.android.media.opensladuio;

public class PlayStatus {

    static {
        System.loadLibrary("native-lib");
    }

    public void sendPlayStatus(String msg, int code) {
        if (playStatus != null) {
            playStatus.playStatus(msg, code);
        }


    }

    public void setIPlayStatus(IPlayStatus playStatus) {
        this.playStatus=playStatus;

    }

    public IPlayStatus playStatus;


    public interface IPlayStatus {
        void playStatus(String msg, int code);
    }


    public native void n_stop();

    public native void n_record(String path);

    public native void n_play(String path);


}
