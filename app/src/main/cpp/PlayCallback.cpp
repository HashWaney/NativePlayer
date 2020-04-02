//
// Created by 王庆 on 2020-04-02.
//

#include "PlayCallback.h"


PlayCallback::PlayCallback(JavaVM *javaVM, const _jobject *jobject1) {
    this->jobj = jobject1;
    this->vm = javaVM;

}

void PlayCallback::callMethod() {

    LOGE("aaaaa");

    JNIEnv *env;
    vm->AttachCurrentThread(&env, 0);

    jobject job = env->NewLocalRef((jobject) jobj);

    jclass jclazz = env->GetObjectClass(job);

    jmethodID jmid = env->GetMethodID(jclazz, "sendPlayStatus", "(Ljava/lang/String;I)V");

    jstring msg = env->NewStringUTF("finish");
    jint code = 100;
    env->CallVoidMethod(job, jmid, msg, code);
    env->DeleteLocalRef(msg);
    env->DeleteLocalRef(job);


    vm->DetachCurrentThread();


    //TODO 此处的env 需要和线程绑定，
    // 如果直接使用 会崩溃，因此需要和JavaVM attach一个当前线程，获取当前线程的 env
    // ，参数jobject 是传入一个全局引用，可以搭配当前线程的env 来创建class method。调用java方法


//    if (env == NULL) {
//        LOGE("jniEnv==NUll ")
//        return;
//    }
//
////    jobject jobj1 = env->NewLocalRef(jobj);
//    if (jobj == NULL) {
//        LOGE("new loca ref is null")
//        return;;
//    }
//
//
//    LOGE("11123333434");
//
//    jobject jinstance = env->NewLocalRef((jobject) jobj);
//
//    jclass jclazz = (env)->GetObjectClass(jinstance);
//    LOGE("21232114214");
//    jmethodID jmethodID1 = env->GetMethodID(jclazz, "sendPlayStatus", "(Ljava/lang/String;I)V");
//    LOGE("12314331231");
//
//    jstring msg = env->NewStringUTF("已经完成播放了");
//    jint code = 111;
//    env->CallVoidMethod(jinstance, jmethodID1, msg, code);
//    env->DeleteLocalRef(msg);
//    env->DeleteLocalRef(jinstance);



//
//    jclass jclazz = jniEnv->GetObjectClass(jobj);
//    jmethodID jmid = jniEnv->GetMethodID(jclazz, "sendPlayStatus", "(Ljava/lang/String;I)V");
//    jniEnv->CallVoidMethod(jobj, jmid, "已经完成播放了", 1111);
}
