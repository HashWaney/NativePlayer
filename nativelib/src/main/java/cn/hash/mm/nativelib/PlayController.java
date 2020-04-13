package cn.hash.mm.nativelib;

import android.os.SystemClock;

/**
 * Created by Hash on 2020-04-13.
 */


public class PlayController {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avdevice");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }

    //播放源
    private static String resource;


    public String getResource() {
        return resource;
    }

    public void setResource(String resource) {
        this.resource = resource;
    }


    public PlayController() {

    }


    //1.准备阶段

    public void prepare() {

    }

    //2.开始播放
    public void startPlay() {

    }


    //3.暂停
    public void pausePlay() {

    }

    //4.恢复
    public void resumePlay() {

    }

    //5.停止
    public void stopPlay() {

    }


}
