package com.android.media.pthreaddemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private ThreadHandler handler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        handler =new ThreadHandler();

    }


    //C++创建线程
    public void pthreadcreate(View view) {
        handler.normalCreateThread();
    }

    //C++加锁 生产者消费者模型构建
    public void mutexthread(View view) {
        handler.mutexThread();
    }

    //C++回调
    public void cpluscallback(View view) {
        handler.callbackFromCplus();
    }
}
