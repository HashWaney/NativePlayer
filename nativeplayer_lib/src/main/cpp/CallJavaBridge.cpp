//
// Created by 王庆 on 2020-03-30.
//

#include <cstddef>
#include "CallJavaBridge.h"

CallJavaBridge::CallJavaBridge(_JavaVM *javaVM, JNIEnv *evn, jobject *jobject) {

    this->javaVM = javaVM;
    this->env = evn;
    this->jobj = *jobject; // * 指针 取内容 也就是取了jobject的内容
    this->jobj = env->NewGlobalRef(jobj); // 通过JNIEnv 实例化一个全局引用jobj


    jclass jclazz = env->GetObjectClass(jobj);
    if (!jclazz) {
        LOGD("jclazz init error ")
        return;
    }

    jmethoid_onprepare = env->GetMethodID(jclazz, "callPrepared", "()V");
    jmethod_playstatus = env->GetMethodID(jclazz, "loadingStatus", "(Z)V");


}

CallJavaBridge::~CallJavaBridge() {

}

void CallJavaBridge::callPrepared(int type) {
    if (MAIN_THREAD == type) {
        env->CallVoidMethod(jobj, jmethoid_onprepare);

    } else if (CHILD_THREAD == type) {

        JNIEnv *env;
        if (JNI_OK != javaVM->AttachCurrentThread(&env, NULL)) {
            return;;
        }

        env->CallVoidMethod(jobj, jmethoid_onprepare);
        javaVM->DetachCurrentThread();
    }


}

void CallJavaBridge::callLoadStatus(bool isLoading, int type) {
    LOGD("callLoadStatus ---->");

    if (MAIN_THREAD == type) {
        env->CallVoidMethod(jobj, jmethod_playstatus, isLoading);

    } else {
        JNIEnv *env;
        if (JNI_OK != javaVM->AttachCurrentThread(&env, NULL)) {
            LOGD("current thread attach fail");
            return;
        } else {
            env->CallVoidMethod(jobj, jmethod_playstatus, isLoading);
            javaVM->DetachCurrentThread();

        }

    }


}

//void CallJavaBridge::loadStatus(bool isPlay, int type) {
//    if (MAIN_THREAD == type) {
//        env->CallVoidMethod(jobj, jmethod_playstatus);
//
//    } else if (CHILD_THREAD == type) {
//        JNIEnv *evn;
//        if (JNI_OK != javaVM->AttachCurrentThread(&evn, NULL)) {
//            return;
//        }
//
//        env->CallVoidMethod(jobj, jmethod_playstatus,isPlay);
//        javaVM->DetachCurrentThread();
//
//
//    }
//
//}

