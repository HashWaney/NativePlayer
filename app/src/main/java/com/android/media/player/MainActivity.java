package com.android.media.player;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.android.media.nativeplayerlib.AudioPlayer;
import com.android.media.nativeplayerlib.TestJNI;
import com.android.media.nativeplayerlib.inter.ILoadingStatus;

public class MainActivity extends AppCompatActivity {

    private String url = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";
//    private String url = Environment.getExternalStorageDirectory().getAbsolutePath() + "/music11.m4u";

    private boolean isPlaying = false;
    private AudioPlayer audioPlayer = null;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        audioPlayer = new AudioPlayer();
        audioPlayer.setUrl(url);
        audioPlayer.setICallNativePrepared(new AudioPlayer.ICallNativePrepared() {
            @Override
            public void prepared() {
                // TODO: 2020-03-30  C++ 解码工作实现完成了，通过调用 Java 层定义的callPrepared方法来告诉外界调用者，
                // 该结果回调是nativeplayerlib库实现的回调，是将C++层调用了库的方法之后，完成的之后的结果告知调用者，
                Log.e(MainActivity.this.getClass().getSimpleName(), "C++ 层已经完成了准备工作了，请进行解码操作");
                audioPlayer.start();
                isPlaying = true;
            }
        });
//        audioPlayer.setLoadingStatus(new ILoadingStatus() {
//            @Override
//            public void play(boolean isLoading) {
//                if (isLoading) {
//                    Log.i("HASH", "数据加载中");
//
//                }else{
//                    Log.i("HASH","数据加载完成。准备播放");
//                }
//            }
//        });
        audioPlayer.setLoadingStatus(new ILoadingStatus() {
            @Override
            public void play(boolean isLoading) {
                if (isLoading) {
                    Log.e("HASH", "准备加载中");
                }
            }
        });

    }


    //进行解码之前的准备阶段 --> 进行解码工作
    public void prepared(View view) {
        Log.i("HASH", "prepared");
        audioPlayer.prepared();

    }

    public void paused(View view) {
        if (isPlaying) {
            audioPlayer.pauseMusic();
            isPlaying = false;
        }
    }

    public void replay(View view) {
        if (!isPlaying) {
            audioPlayer.replayMusic();
            isPlaying = true;

        }
    }
}
