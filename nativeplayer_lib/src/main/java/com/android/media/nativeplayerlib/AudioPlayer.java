package com.android.media.nativeplayerlib;

import java.nio.file.Path;

/**
 * 音频播放器
 */
public class AudioPlayer {


    private ICallNativePrepared iCallNativePrepared;
    private String url;

    static {
        System.loadLibrary("native-lib");

        //libavcodec.so
        System.loadLibrary("avcodec");

//        libavdevice.so

        System.loadLibrary("avdevice");

        //libavfilter.so
        System.loadLibrary("avfilter");

        //libavformat.so
        System.loadLibrary("avformat");

        //libavutil.so
        System.loadLibrary("avutil");

        //libswresample.so
        System.loadLibrary("swresample");

        //libswscale.so
        System.loadLibrary("swscale");

    }


    public void setUrl(String url) {
        this.url = url;
    }

    public native void prepared(String url);

    public native void startDecode();


    //C++ 调用

    public void callPrepared() {
        if (iCallNativePrepared != null) {
            iCallNativePrepared.prepared();
        }


    }


    public interface ICallNativePrepared {
        void prepared();
    }

    public void setICallNativePrepared(ICallNativePrepared iCallNativePrepared) {
        this.iCallNativePrepared = iCallNativePrepared;
    }


}
