/**
 * Created by Hash on 2020-04-14.
 */


#ifndef AUDIOPLAYER_JAVABRIDGE_H
#define AUDIOPLAYER_JAVABRIDGE_H

#include "jni.h"
#include "../log/AudioLog.h"


/**
 *  TODO 因为这里涉及到了线程问题，JNIEnv 是和线程绑定的，
 *  如果单纯是使用主线程的JNIEnv没有问题，但是子线程就有问题了。因此这里定义常量来区分是主线程还是子线程
 */

#define MAIN_THREAD 1
#define TASK_THREAD 2


class JavaBridge {
public:
    _JavaVM *vm = NULL;
    JNIEnv *env = NULL;
    jobject instance;
    jmethodID method_prepared; // 准备的 回调
    jmethodID method_load; // 加载的回调
    jmethodID method_timeInfo; //播放时长的回调
    jmethodID method_errorMessage; //错误信息的回调
    jmethodID method_complete; //播放完成的回调
    jmethodID method_db;//db获取
public:
    JavaBridge(_JavaVM *javaVM, JNIEnv *env, jobject *jobj);

    ~JavaBridge();


    void onCallPrepared(int type);

    void onCallTimeInfo(int type, int currentTime, int totalTime);

    void onCallErrMessage(int type, int errCode, const char *errMessage);


    void onCallLoad(int type, bool isLoad);

    void onCallComplete(int type, bool isComplete);

    void onCallVolumeDb(int type, int db);
};


#endif //AUDIOPLAYER_JAVABRIDGE_H
