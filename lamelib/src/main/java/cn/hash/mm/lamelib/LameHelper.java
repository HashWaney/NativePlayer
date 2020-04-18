package cn.hash.mm.lamelib;

/**
 * Created by Hash on 2020-04-18.
 */


public class LameHelper {

    static {
        System.loadLibrary(" lame-lib");
    }

    public static void printLameVersion() {
        System.err.println(getVersion());
    }

    public static native String getVersion();
}
