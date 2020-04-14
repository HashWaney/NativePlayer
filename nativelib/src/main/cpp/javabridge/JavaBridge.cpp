/**
 * Created by Hash on 2020-04-14.
 */


#include "JavaBridge.h"

JavaBridge::JavaBridge(_JavaVM *javaVM, JNIEnv *env, jobject *jobj) {
    this->vm = javaVM;
    this->env = env;
    this->instance = *jobj;
    this->instance = env->NewGlobalRef(*jobj);

    jclass java_clazz = env->GetObjectClass(instance);
    if (!java_clazz) {
        LOG_E("can not find the java instance class");
        return;
    }
    method_prepared = env->GetMethodID(java_clazz, "prepareCallBackFormNative", "()V");

}

JavaBridge::~JavaBridge() {

}

//1: after decode the stream ,then to tell the user now can to play
void JavaBridge::onCallPrepared(int type) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(instance, method_prepared);
    } else if (type == TASK_THREAD) {
        JNIEnv *jniEnv;
        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOG_E("the jniEnv can not bind the task thread");
            return;
        }

        jniEnv->CallVoidMethod(instance, method_prepared);

        vm->DetachCurrentThread();

    }
}