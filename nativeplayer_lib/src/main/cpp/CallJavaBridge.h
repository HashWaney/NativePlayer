//
// Created by 王庆 on 2020-03-30.
//

#ifndef PLAYER_CALLJAVABRIDGE_H
#define PLAYER_CALLJAVABRIDGE_H

#include "jni.h"
#include "AndroidLog.h"
#define MAIN_THREAD  0
#define CHILD_THREAD 1
class CallJavaBridge {
public:
    _JavaVM *javaVM = NULL;
    JNIEnv *env = NULL;
    jobject jobj;
    jmethodID jmethodId;


public:
    CallJavaBridge(_JavaVM *javaVM, JNIEnv *evn, jobject *jclazz);

    ~CallJavaBridge();

    //C++完成解码操作，将结果通知Java层调用者
    void callPrepared(int type);

};


#endif //PLAYER_CALLJAVABRIDGE_H
