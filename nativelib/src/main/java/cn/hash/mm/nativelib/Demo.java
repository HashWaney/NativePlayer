package cn.hash.mm.nativelib;

import android.os.SystemClock;

/**
 * Created by Hash on 2020-04-13.
 */


public class Demo {

    static {
        System.loadLibrary("native-lib");
    }

    public static void test() {
        System.err.println("test lib" + stringFromJNI());

    }

    public static native String stringFromJNI();
}
