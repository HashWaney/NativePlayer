package cn.hash.mm.audioplayer;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import java.io.File;

import cn.hash.mm.audioplayer.util.LogUtil;
import cn.hash.mm.audioplayer.util.PermissionUtil;
import cn.hash.mm.nativelib.PlayController;
import cn.hash.mm.nativelib.listener.OnPrepareListener;

/**
 * Created by Hash on 2020-04-13.
 */


public class MainActivity extends AppCompatActivity {

    private PlayController playController;

    private String url = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "fyjili.mp3";
    private String net_url = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        PermissionUtil.requestPermission(this);
        playController = new PlayController();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        PermissionUtil.onRequestPermissionResult(requestCode, permissions, grantResults);
    }

    public void stop(View view) {
    }


    public void resume(View view) {
    }

    public void pause(View view) {
    }

    public void start(View view) {
//        Demo.test();
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
}
