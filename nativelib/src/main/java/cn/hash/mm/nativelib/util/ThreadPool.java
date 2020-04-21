package cn.hash.mm.nativelib.util;

import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

/**
 * Created by Hash on 2020-04-13.
 */


public class ThreadPool {
    private static Executor mExecutor = Executors.newCachedThreadPool();

    public static void execute(Runnable task) {
        mExecutor.execute(task);
    }


}
