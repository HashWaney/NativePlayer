//
// Created by 王庆 on 2020-03-30.
//

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
    jmethodId = env->GetMethodID(jclazz, "callPrepared", "()V");


//    jclass clazz = evn->GetObjectClass(instance);
//    //TODO callPrepared 方法是在Java中声明的
//    jmethodId = env->GetMethodID(clazz, "callPrepared", "()V");
//    env->CallVoidMethod(instance, jmethodId);

}

CallJavaBridge::~CallJavaBridge() {

}

void CallJavaBridge::callPrepared(int type) {
    if (MAIN_THREAD == type) {
        env->CallVoidMethod(jobj, jmethodId);

    } else if (CHILD_THREAD == type) {
        JNIEnv *jniEnv;

        if(javaVM->AttachCurrentThread(&jniEnv, NULL) !=JNI_OK){
            LOGD("jniEnv find error");
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmethodId);

        javaVM->DetachCurrentThread();

    }


}
