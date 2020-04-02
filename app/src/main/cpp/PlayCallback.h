//
// Created by 王庆 on 2020-04-02.
//

#ifndef OPENSLADUIO_PLAYCALLBACK_H
#define OPENSLADUIO_PLAYCALLBACK_H

#include "android/log.h"
#include "jni.h"

#define TAG "HASH"
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);

class PlayCallback {
public:
    JavaVM *vm;
    const _jobject *jobj;

public:
    PlayCallback(JavaVM *javaVM, const _jobject *jobj);

    ~PlayCallback();

    void callMethod();

};


#endif //OPENSLADUIO_PLAYCALLBACK_H
