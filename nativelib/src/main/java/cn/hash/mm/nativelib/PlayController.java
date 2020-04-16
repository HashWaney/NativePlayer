package cn.hash.mm.nativelib;

import android.os.SystemClock;
import android.text.TextUtils;
import android.util.Log;

import androidx.core.app.NavUtils;

import cn.hash.mm.nativelib.bean.AudioInfoBean;
import cn.hash.mm.nativelib.listener.OnPauseResumeListener;
import cn.hash.mm.nativelib.listener.OnPlayCompleteListener;
import cn.hash.mm.nativelib.listener.OnPlayErrorListener;
import cn.hash.mm.nativelib.listener.OnPlayLoadListener;
import cn.hash.mm.nativelib.listener.OnPrepareListener;
import cn.hash.mm.nativelib.listener.OnPrepareLoadListener;
import cn.hash.mm.nativelib.listener.OnTimeInfoListener;

/**
 * Created by Hash on 2020-04-13.
 */


public class PlayController {

    private static final String TAG = PlayController.class.toString();

    private OnPrepareListener onPrepareListener;

    private OnPlayLoadListener onPlayLoadListener;

    private OnPauseResumeListener onPauseResumeListener;

    private OnTimeInfoListener timeInfoListener;

    private OnPlayErrorListener onPlayErrorListener;

    private OnPlayCompleteListener onPlayCompleteListener;

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

    private static AudioInfoBean audioInfoBean = null;


    public String getResource() {
        return resource;
    }

    public void setResource(String resource) {
        this.resource = resource;
    }

    public void setOnPrepareListener(OnPrepareListener prepareListener) {
        this.onPrepareListener = prepareListener;
    }

    public void setOnPauseResumeListener(OnPauseResumeListener onPauseResumeListener) {
        this.onPauseResumeListener = onPauseResumeListener;
    }


    public void setTimeInfoListener(OnTimeInfoListener timeInfoListener) {
        this.timeInfoListener = timeInfoListener;
    }

    public void setOnPlayErrorListener(OnPlayErrorListener errorListener) {
        this.onPlayErrorListener = errorListener;
    }

    public void setOnPlayLoadListener(OnPlayLoadListener onPlayLoadListener) {
        this.onPlayLoadListener = onPlayLoadListener;
    }

    public void setOnPlayCompleteListener(OnPlayCompleteListener onPlayCompleteListener) {
        this.onPlayCompleteListener = onPlayCompleteListener;
    }

    public PlayController() {

    }


    //1.准备阶段

    public void prepare() {
        if (TextUtils.isEmpty(resource)) {
            Log.d(TAG, String.format("url %d is null", resource));
            return;
        }


        //开启一个子线程
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepare(resource);
            }
        }).start();


    }

    //2.开始播放
    public void startPlay() {
        if (TextUtils.isEmpty(resource)) {
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_startPlay();
            }
        }).start();

    }


    //3.暂停
    public void pausePlay() {
        n_pause();
        if (onPauseResumeListener != null) {
            onPauseResumeListener.onPauseOrResume(true);
        }

    }

    //4.恢复
    public void resumePlay() {
        n_resume();
        if (onPauseResumeListener != null) {
            onPauseResumeListener.onPauseOrResume(false);
        }

    }


    //5.停止
    public void stopPlay() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();


    }

    //6.进度
    public void seek(int second) {
        n_seek(second);
    }

    ///////////////////native callback////////////////////////////////////
    public void prepareCallBackFormNative() {
        if (onPrepareListener != null) {
            onPrepareListener.prepare();
        }

    }


    public void callLoadFromNative(boolean isLoad) {
        if (onPlayLoadListener != null) {
            onPlayLoadListener.onLoad(isLoad);

        }
    }

    public void callCompleteFromNative(boolean isComplete) {
        if (onPlayCompleteListener != null) {
            onPlayCompleteListener.complete(isComplete);
        }
    }

    //获取当前播放时长和总时长
    public void callTimeInfoFromNative(int currentTime, int totalTime) {
        if (timeInfoListener != null) {
            if (audioInfoBean == null) {
                audioInfoBean = new AudioInfoBean();
            }
            audioInfoBean.setCurrentTime(currentTime);
            audioInfoBean.setTotalTime(totalTime);
            timeInfoListener.getCurrentTimeInfo(audioInfoBean);
        }
    }


    public void errMessageFromNative(int errorCode, String errorMessage) {
        if (onPlayErrorListener != null) {
            onPlayErrorListener.onError(errorCode, errorMessage);
        }
    }

    public void callPlayCompleteFromNative(boolean isComplete) {
        if (onPlayCompleteListener != null) {
            onPlayCompleteListener.complete(isComplete);
        }

    }


    /////////////////////////native///////////////////////////////////
    public native void n_prepare(String resource);

    public native void n_startPlay();

    public native void n_pause();

    public native void n_resume();

    public native void n_stop();

    public native void n_seek(int seconds);


}
