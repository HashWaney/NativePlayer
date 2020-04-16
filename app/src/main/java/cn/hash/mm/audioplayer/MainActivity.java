package cn.hash.mm.audioplayer;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
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
import cn.hash.mm.nativelib.listener.OnPlayCompleteListener;
import cn.hash.mm.nativelib.listener.OnPlayErrorListener;
import cn.hash.mm.nativelib.listener.OnPlayLoadListener;
import cn.hash.mm.nativelib.listener.OnPrepareListener;
import cn.hash.mm.nativelib.listener.OnTimeInfoListener;
import cn.hash.mm.nativelib.util.TimeUtil;

/**
 * Created by Hash on 2020-04-13.
 */


public class MainActivity extends AppCompatActivity implements OnPauseResumeListener, OnTimeInfoListener, OnPlayErrorListener, SeekBar.OnSeekBarChangeListener, OnPlayLoadListener, OnPlayCompleteListener, OnPrepareListener {

    private PlayController playController;
    private boolean isPrepared = false;
    private boolean isPlaying = false;
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
        playController.setOnPlayCompleteListener(this);
        playController.setOnPlayLoadListener(this);
        playController.setTimeInfoListener(this);
        playController.setOnPlayErrorListener(this);
        playController.setOnPrepareListener(this);
        seekBar.setOnSeekBarChangeListener(this);

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        PermissionUtil.onRequestPermissionResult(requestCode, permissions, grantResults);
    }


    public void start(View view) {
        prepared(url);

    }

    public void prepared(String url) {
        LogUtil.logD("执行prepared 方法");

        playController.setResource(url);
        playController.prepare();
    }


    public void resume(View view) {
        if (!isPlaying) {
            playController.resumePlay();

        }

    }

    public void pause(View view) {
        if (isPlaying) {
            playController.pausePlay();

        }
    }

    public void stop(View view) {
        isPrepared = false;
        playController.stopAndRelease(-1);
    }


    @Override
    public void onPauseOrResume(boolean isPause) {
        isPrepared = false;
        isPlaying = !isPause;
        LogUtil.logD(isPause ? "pause to play:" + Thread.currentThread().getName() : "resume to play:" + Thread.currentThread().getName());
    }


    @Override
    public void getCurrentTimeInfo(AudioInfoBean audioInfoBean) {
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
        playController.seek(115);
    }

    @Override
    public void onLoad(boolean isLoad) {
        LogUtil.logD(isLoad ? "加载中..." : "播放中...");
        isPrepared = isLoad;
        isPlaying = !isLoad;

    }

    @Override
    public void complete(boolean isComplete) {
        if (isComplete) {
            LogUtil.logD("播放完成");
            //1，释放资源
            isPrepared = false;
            isPlaying = false;
            //auto play next after the music is complete .
            playController.playNext(net_url);

        }

    }

    @Override
    public void prepare() {
        LogUtil.logD("解码已完成，准备进行播放吧");
        playController.startPlay();
    }

    public void playnext(View view) {
        playController.playNext(net_url);

    }
}
