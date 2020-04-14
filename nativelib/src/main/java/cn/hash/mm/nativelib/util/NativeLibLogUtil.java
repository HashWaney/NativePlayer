package cn.hash.mm.nativelib.util;

import android.util.Log;

/**
 * Created by Hash on 2020-04-14.
 */


public class NativeLibLogUtil {
    private static final String TAG = NativeLibLogUtil.class.toString();


    public static void logE(String msg) {
        Log.e(TAG, msg);
    }


    public static void logW(String msg) {
        Log.w(TAG, msg);
    }


    public static void logI(String msg) {
        Log.i(TAG, msg);
    }

    public static void logD(String msg) {
        Log.d(TAG, msg);
    }
}
