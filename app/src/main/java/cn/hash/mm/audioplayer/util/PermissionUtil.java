package cn.hash.mm.audioplayer.util;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import static androidx.core.content.ContextCompat.checkSelfPermission;

/**
 * Created by Hash on 2020-04-13.
 */


public class PermissionUtil {

    private static final String TAG = PermissionUtil.class.toString();
    // 读写权限组
    public static final String[] requestStoragePermission = {
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };
    private static Context context;

    public static final int STORE_REQUEST_CODE = 10001;

    public static void setContext(Context ctx) {
        context = ctx;
    }

    public static boolean checkHasStoragePermission(Context ctx) {
        //6.0以上
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            return ActivityCompat.checkSelfPermission(ctx, Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        }

        return true;
    }

    public static void requestPermission(Context context) {
        Log.w(TAG, "context :" + context);
        if (!checkHasStoragePermission(context)) {
            if (context instanceof AppCompatActivity) {
                ActivityCompat.requestPermissions(((AppCompatActivity) context), requestStoragePermission, STORE_REQUEST_CODE);
            }
        }
    }


    public static void onRequestPermissionResult(int requestCode, String[] permissions, int[] grantResults) {
        Log.w(TAG, "requestCode:" + requestCode + " permission:" + permissions + " grantResults:" + grantResults);
        if (requestCode == STORE_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(context, "授权成功", Toast.LENGTH_SHORT).show();
            }
        }

    }
}
