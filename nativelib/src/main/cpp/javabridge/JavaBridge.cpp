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
    method_timeInfo = env->GetMethodID(java_clazz, "callTimeInfoFromNative", "(II)V");
    method_errorMessage = env->GetMethodID(java_clazz, "errMessageFromNative",
                                           "(ILjava/lang/String;)V");

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

void JavaBridge::onCallTimeInfo(int type, int currentTime, int totalTime) {
    if (type == MAIN_THREAD) {

        env->CallVoidMethod(instance, method_timeInfo, currentTime, totalTime);
    } else {
        JNIEnv *jniEnv;
        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            LOG_E("call time info error");
            return;
        }
        jniEnv->CallVoidMethod(instance, method_timeInfo, currentTime, totalTime);
        vm->DetachCurrentThread();

    }

}

void JavaBridge::onCallErrMessage(int type, int errCode, char *errMessage) {
    if (type == MAIN_THREAD) {
        jstring message = env->NewStringUTF(errMessage);
        env->CallVoidMethod(instance, method_errorMessage, errCode, message);
        env->DeleteLocalRef(message);

    } else {
        JNIEnv *jniEnv;
        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jstring message = jniEnv->NewStringUTF(errMessage);
        jniEnv->CallVoidMethod(instance, method_errorMessage, errCode, message);
        jniEnv->DeleteLocalRef(message);
        vm->DetachCurrentThread();

    }

}


