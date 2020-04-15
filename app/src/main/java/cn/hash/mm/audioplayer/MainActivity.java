package cn.hash.mm.audioplayer;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatSeekBar;

import java.io.File;

import cn.hash.mm.audioplayer.util.LogUtil;
import cn.hash.mm.audioplayer.util.PermissionUtil;
import cn.hash.mm.nativelib.PlayController;
import cn.hash.mm.nativelib.bean.AudioInfoBean;
import cn.hash.mm.nativelib.listener.OnPauseResumeListener;
import cn.hash.mm.nativelib.listener.OnPlayErrorListener;
import cn.hash.mm.nativelib.listener.OnPrepareListener;
import cn.hash.mm.nativelib.listener.OnPrepareLoadListener;
import cn.hash.mm.nativelib.listener.OnTimeInfoListener;
import cn.hash.mm.nativelib.util.TimeUtil;

/**
 * Created by Hash on 2020-04-13.
 */


public class MainActivity extends AppCompatActivity implements OnPauseResumeListener, OnPrepareLoadListener, OnTimeInfoListener, OnPlayErrorListener, SeekBar.OnSeekBarChangeListener {

    private PlayController playController;
    private boolean isPrepared = false;
    private TextView tvPlayTime;

    private AppCompatSeekBar seekBar;

    private Handler timeInfoHandler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            // TODO: 2020-04-15
            switch (msg.what) {
                case 1:
                    AudioInfoBean audioInfoBean = (AudioInfoBean) msg.obj;
                    tvPlayTime.setText(TimeUtil.secToDataFormat(audioInfoBean.getCurrentTime(), audioInfoBean.getTotalTime()) + "/" +
                            TimeUtil.secToDataFormat(audioInfoBean.getTotalTime(), audioInfoBean.getTotalTime()));
                    break;
            }
        }
    };


    private String url = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "fyjili.mp3";
    private String net_url = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        PermissionUtil.requestPermission(this);
        tvPlayTime = findViewById(R.id.tvPlayTime);
        seekBar = findViewById(R.id.seek);
        playController = new PlayController();
        playController.setOnPauseResumeListener(this);
        playController.setOnPrepareLoadListener(this);
        playController.setTimeInfoListener(this);
        playController.setOnPlayErrorListener(this);
        seekBar.setOnSeekBarChangeListener(this);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        PermissionUtil.onRequestPermissionResult(requestCode, permissions, grantResults);
    }

    public void stop(View view) {
        isPrepared = false;
        playController.stopPlay();
    }


    public void resume(View view) {
        playController.resumePlay();

    }

    public void pause(View view) {
        playController.pausePlay();
    }

    public void start(View view) {
        if (isPrepared) {
            return;
        }
        playController.setResource(url);
        playController.prepare();
        playController.setOnPrepareListener(new OnPrepareListener() {
            @Override
            public void prepare() {
                LogUtil.logD("解码已完成，准备进行播放吧");
                playController.startPlay();
            }
        });
    }

    @Override
    public void onPauseOrResume(boolean isPause) {
        isPrepared = true;
        LogUtil.logD(isPause ? "pause to play:" + Thread.currentThread().getName() : "resume to play:" + Thread.currentThread().getName());
    }

    @Override
    public void onPrepareLoad(boolean isLoad) {
        isPrepared = true;
        LogUtil.logD(isLoad ? "加载中..." : "播放中...");
    }

    @Override
    public void getCurrentTimeInfo(AudioInfoBean audioInfoBean) {
        LogUtil.logFormat(TimeUtil.secToDataFormat(audioInfoBean.getCurrentTime(), audioInfoBean.getTotalTime()) + "/" +
                TimeUtil.secToDataFormat(audioInfoBean.getTotalTime(), audioInfoBean.getTotalTime()));
        Message message = Message.obtain();
        message.what = 1;
        message.obj = audioInfoBean;
        timeInfoHandler.sendMessage(message);
    }

    @Override
    public void onError(int errorCode, String errorMessage) {
        LogUtil.logFormat(errorMessage + "/ code :" + errorCode);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        LogUtil.logD("progress:" + progress);

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        LogUtil.logD("start touch");
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        LogUtil.logD("stop touch");

    }

    public void seek(View view) {
        playController.seek(111);
    }
}
