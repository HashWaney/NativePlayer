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


    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


    }

    /**
     * 我们要通过麦克风录制一段音频，音频格式为pcm 文件名称为temp.pcm 路径为/sdcard/temp.pcm
     * @param view
     */
    public void recordSound(View view) {
        Log.i("HASH", "path:" + Environment.getExternalStorageDirectory().getAbsolutePath());


        n_record(Environment.getExternalStorageDirectory().getAbsolutePath() + "/temp.pcm");

    }

    public void stopRecord(View view) {

        n_stop();
    }

    public native void n_stop();

    public native void n_record(String path);


}




