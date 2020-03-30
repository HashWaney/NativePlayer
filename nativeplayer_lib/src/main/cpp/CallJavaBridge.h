//
// Created by 王庆 on 2020-03-30.
//

#ifndef PLAYER_CALLJAVABRIDGE_H
#define PLAYER_CALLJAVABRIDGE_H

#include "jni.h"

class CallJavaBridge {
public:
    _JavaVM *javaVM = NULL;
    JNIEnv *env = NULL;
    jobject jclazz;
    jmethodID jmethodId;


public:
    CallJavaBridge(_JavaVM *javaVM, JNIEnv *evn, jobject jclazz);

    ~CallJavaBridge();


};


#endif //PLAYER_CALLJAVABRIDGE_H
