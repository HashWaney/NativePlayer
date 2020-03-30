package com.android.media.player;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.android.media.nativeplayerlib.AudioPlayer;
import com.android.media.nativeplayerlib.TestJNI;

public class MainActivity extends AppCompatActivity {

    private String url = "http://music.163.com/song/media/outer/url?id=281951.mp3";

    private AudioPlayer audioPlayer = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        audioPlayer = new AudioPlayer();
        audioPlayer.setICallNativePrepared(new AudioPlayer.ICallNativePrepared() {
            @Override
            public void prepared() {
                // TODO: 2020-03-30  C++ 解码工作实现完成了，通过调用 Java 层定义的callPrepared方法来告诉外界调用者，
                // 该结果回调是nativeplayerlib库实现的回调，是将C++层调用了库的方法之后，完成的之后的结果告知调用者，
                Log.e(MainActivity.this.getClass().getSimpleName(), "C++ 层已经完成了解码工作了，请进行播放操作");

                audioPlayer.startPlay();
            }
        });

    }


    //进行解码之前的准备阶段 --> 进行解码工作
    public void prepared(View view) {
        audioPlayer.prepared(url);

    }
}
