package cn.hash.mm.audioplayer;

import android.os.Bundle;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import cn.hash.mm.audioplayer.util.PermissionUtil;
import cn.hash.mm.nativelib.Demo;

/**
 * Created by Hash on 2020-04-13.
 */


public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        PermissionUtil.requestPermission(this);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        PermissionUtil.onRequestPermissionResult(requestCode,permissions,grantResults);
    }

    public void stop(View view) {
    }


    public void resume(View view) {
    }

    public void pause(View view) {
    }

    public void start(View view) {
        Demo.test();
    }
}
