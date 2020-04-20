package cn.hash.mm.audioplayer;

import android.media.AudioManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatSeekBar;

import cn.hash.mm.audioplayer.util.LogUtil;
import cn.hash.mm.audioplayer.util.PermissionUtil;

import cn.hash.mm.nativelib.PlayController;
import cn.hash.mm.nativelib.bean.AudioInfoBean;
import cn.hash.mm.nativelib.bean.MuteType;
import cn.hash.mm.nativelib.listener.OnCurrentAudioDbListener;
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


public class MainActivity extends AppCompatActivity implements OnPauseResumeListener, OnTimeInfoListener, OnPlayErrorListener, SeekBar.OnSeekBarChangeListener, OnPlayLoadListener, OnPlayCompleteListener, OnPrepareListener, OnCurrentAudioDbListener {

    private PlayController playController;
    private boolean isPlaying = false;
    private TextView tvPlayTime, tvVolume, tvDb;
    protected boolean isSeekPosition = false;
    private int currentPosition = 0;
    private AppCompatSeekBar volumeSeekBar, positionSeekBar;

    private AudioManager audioManager;


    private static final int defaultVolume = 40;

    private int stepVolume = 0;

    private int currentVolume = 0;

    private Handler timeInfoHandler = new Handler() {
        @Override
        public void handleMessage(@NonNull Message msg) {
            // TODO: 2020-04-15
            switch (msg.what) {
                case 1:
                    if (!isSeekPosition) {
                        AudioInfoBean audioInfoBean = (AudioInfoBean) msg.obj;
                        tvPlayTime.setText(TimeUtil.secToDataFormat(audioInfoBean.getCurrentTime(), audioInfoBean.getTotalTime()) + "/" +
                                TimeUtil.secToDataFormat(audioInfoBean.getTotalTime(), audioInfoBean.getTotalTime()));
                        //update the progress
                        positionSeekBar.setProgress(audioInfoBean.getCurrentTime() * 100 / audioInfoBean.getTotalTime());
                    }

                    break;


            }
        }
    };


    private String net_url = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "fyjili.mp3";

    private String url = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";
    private String broadcast_url = "http://ngcdn004.cnr.cn/live/dszs/index12.m3u8";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        PermissionUtil.requestPermission(this);
        tvPlayTime = findViewById(R.id.tvPlayTime);
        tvVolume = findViewById(R.id.tvVolume);
        volumeSeekBar = findViewById(R.id.volume_seek);
        tvDb = findViewById(R.id.tvDb);
        positionSeekBar = findViewById(R.id.position_seek);


        playController = new PlayController();
        playController.setVolume(defaultVolume);
        playController.setMuteType(MuteType.MUTE_TYPE_CENTER);
        playController.setPitch(1.0f);
        playController.setSpeed(1.0f);
        volumeSeekBar.setProgress(playController.getVolume());
        tvVolume.setText("音量：" + playController.getVolume());
        playController.setOnPauseResumeListener(this);
        playController.setOnPlayCompleteListener(this);
        playController.setOnPlayLoadListener(this);
        playController.setTimeInfoListener(this);
        playController.setOnPlayErrorListener(this);
        playController.setOnPrepareListener(this);
        playController.setOnCurrentAudioDbListener(this);
        volumeSeekBar.setOnSeekBarChangeListener(this);
        positionSeekBar.setOnSeekBarChangeListener(this);


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
        }
        playController.resumePlay();


    }

    public void pause(View view) {
        if (isPlaying) {

        }
        playController.pausePlay();
    }

    public void stop(View view) {

        playController.stopAndRelease(-1);
    }


    @Override
    public void onPauseOrResume(boolean isPause) {
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
    public void onError(int errorCode, final String errorMessage) {
        LogUtil.logFormat(errorMessage + "/ code :" + errorCode);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, errorMessage, Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (seekBar.getId() == R.id.position_seek) {
            LogUtil.logD("position Seek progress:" + progress);
            if (playController.getDuration() > 0 && isSeekPosition) {
                currentPosition = playController.getDuration() * progress / 100;
                LogUtil.logFormat("currentPosition:" + currentPosition + " progress: " + progress);
            }


        } else if (seekBar.getId() == R.id.volume_seek) {
            LogUtil.logD("volume Seek :" + progress);
            // TODO: 2020-04-16
            playController.setVolume(progress);
            tvVolume.setText("音量:" + playController.getVolume());


        }

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        LogUtil.logD("start touch");
        if (seekBar.getId() == R.id.position_seek) {
            isSeekPosition = true;
        } else if (seekBar.getId() == R.id.volume_seek) {

        }
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        LogUtil.logD("stop touch");
        if (seekBar.getId() == R.id.position_seek) {
            playController.seek(currentPosition);
            isSeekPosition = false;
        }

    }


    @Override
    public void onLoad(boolean isLoad) {
        LogUtil.logD(isLoad ? "加载中..." : "播放中...");
        isPlaying = !isLoad;

    }

    @Override
    public void complete(boolean isComplete) {
        if (isComplete) {
            LogUtil.logD("播放完成");
            //1，释放资源
            isPlaying = false;
            //auto play next after the music is complete .

            playController.playNext(broadcast_url);
            volumeSeekBar.setProgress(playController.getVolume());

        }

    }

    @Override
    public void prepare() {
        LogUtil.logD("解码已完成，准备进行播放吧");
        playController.startPlay();
    }

    public void playNext(View view) {
        LogUtil.logD("current progress:" + positionSeekBar.getProgress());
        positionSeekBar.setProgress(0);
        positionSeekBar.jumpDrawablesToCurrentState();
        playController.playNext(net_url);
        volumeSeekBar.setProgress(playController.getVolume());


    }


    public void left(View view) {
        playController.setMuteType(MuteType.MUTE_TYPE_LEFT);
    }

    public void right(View view) {
        playController.setMuteType(MuteType.MUTE_TYPE_RIGHT);
    }

    public void center(View view) {
        playController.setMuteType(MuteType.MUTE_TYPE_CENTER);
    }

    public void normal(View view) {
        playController.setSpeed(1.0f);
        playController.setPitch(1.0f);
    }

    public void pitchAndspeed(View view) {
        playController.setPitch(1.5f);
        playController.setPitch(1.5f);
    }

    public void pitch(View view) {
        playController.setPitch(1.5f);
        playController.setSpeed(1.0f);

    }

    public void speed(View view) {
        playController.setSpeed(1.5f);
        playController.setPitch(1.0f);
    }

    @Override
    public void setCurrentDb(final int db) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                tvDb.setText("分贝值:" + db);
            }
        });
    }
}
