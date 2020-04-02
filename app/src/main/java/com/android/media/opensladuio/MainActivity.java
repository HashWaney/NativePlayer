package com.android.media.opensladuio;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {


    private PlayStatus playStatus;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        playStatus = new PlayStatus();
        playStatus.setIPlayStatus(new PlayStatus.IPlayStatus() {
            @Override
            public void playStatus(String msg, int code) {
                // finish:100 current thread；Thread-8 说明回调回来的是子线程，更加验证了 JNIEnv 绑定当前的线程，因此需要通过JNI_OnLoad()来获取JavaVM
                //然后通过JavaVM来attach 当前线程，获取到当前线程的JNIEnv
                Log.i("HASH", msg + ":" + code + " current thread；" + Thread.currentThread().getName());
            }
        });


    }

    /**
     * 我们要通过麦克风录制一段音频，音频格式为pcm 文件名称为temp.pcm 路径为/sdcard/temp.pcm
     *
     * @param view
     */
    public void recordSound(View view) {
        Log.i("HASH", "path:" + Environment.getExternalStorageDirectory().getAbsolutePath());


        playStatus.n_record(Environment.getExternalStorageDirectory().getAbsolutePath() + "/temp.pcm");

    }

    public void stopRecord(View view) {

        playStatus.n_stop();
    }


    public void playPcm(View view) {
        playStatus.n_play(Environment.getExternalStorageDirectory().getAbsolutePath() + "/temp.pcm");
    }
}




