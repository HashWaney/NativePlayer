//
// Created by 王庆 on 2020-03-30.
//

#include "CallJavaBridge.h"

CallJavaBridge::CallJavaBridge(_JavaVM *javaVM, JNIEnv *evn, jobject jclazz) {

//    this->javaVM = javaVM;
//    this->env = evn;
//    this->jclazz = jclazz;
//    jobject instance = evn->NewGlobalRef(jclazz);
//    jclass clazz = evn->GetObjectClass(instance);
//    //TODO callPrepared 方法是在Java中声明的
//    jmethodId = env->GetMethodID(clazz, "callPrepared", "()V");
//    env->CallVoidMethod(instance, jmethodId);

}

CallJavaBridge::~CallJavaBridge() {

}
