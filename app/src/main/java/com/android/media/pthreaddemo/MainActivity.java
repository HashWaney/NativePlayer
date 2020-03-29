package com.android.media.pthreaddemo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private ThreadHandler handler;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        handler = new ThreadHandler();
        handler.setOnCallback(new ThreadHandler.Callback() {
            @Override
            public void onCallback(int type, final String msg) {
                if (TextUtils.isEmpty(msg)) {
                    return;
                }

                Log.i(MainActivity.this.getClass().getSimpleName(), "msg from c++ :" + msg + " type :" + type);
                //C++ 子线程
                if (type == 0) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(MainActivity.this, msg, Toast.LENGTH_LONG).show();
                        }
                    });

                } else {
                    Toast.makeText(MainActivity.this, msg, Toast.LENGTH_SHORT).show();
                }

            }
        });

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
