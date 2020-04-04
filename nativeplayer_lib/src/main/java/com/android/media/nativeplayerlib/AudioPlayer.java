package com.android.media.nativeplayerlib;

import android.text.TextUtils;
import android.util.Log;

import com.android.media.nativeplayerlib.inter.ILoadingStatus;

/**
 * 音频播放器
 */
public class AudioPlayer {


    private ICallNativePrepared iCallNativePrepared;

    private ILoadingStatus loadingStatus;
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

    public native void pauseMusic();

    public native void replayMusic();


    //C++ 调用

    public void callPrepared() {
        if (iCallNativePrepared != null) {
            iCallNativePrepared.prepared();
        }
    }

    //当前是否在加载中
    public void loadingStatus(boolean isLoadding) {
        if (loadingStatus != null) {
            loadingStatus.play(isLoadding);
        }
    }


    public void prepared() {
        Log.i("HASH", "判断URL是否为null：" + TextUtils.isEmpty(url));
        if (TextUtils.isEmpty(url)) {
            Log.i("HASH", "url 为空 不进行解码操作");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                prepared(url);
            }
        }).start();
    }

    public void start() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                startDecode();
            }
        }).start();
    }


    public interface ICallNativePrepared {
        void prepared();
    }

    public void setICallNativePrepared(ICallNativePrepared iCallNativePrepared) {
        this.iCallNativePrepared = iCallNativePrepared;
    }


    public void setLoadingStatus(ILoadingStatus status) {
        this.loadingStatus = status;

    }


}
