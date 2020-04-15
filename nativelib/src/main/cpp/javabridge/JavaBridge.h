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
    jmethodID method_prepared, method_timeInfo;
public:
    JavaBridge(_JavaVM *javaVM, JNIEnv *env, jobject *jobj);

    ~JavaBridge();


    void onCallPrepared(int type);

    void onCallTimeInfo(int type, int currentTime, int totalTime);

};


#endif //AUDIOPLAYER_JAVABRIDGE_H
