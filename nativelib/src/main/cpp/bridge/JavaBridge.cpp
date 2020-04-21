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

    method_load = env->GetMethodID(java_clazz, "callLoadFromNative", "(Z)V");
    method_complete = env->GetMethodID(java_clazz, "callCompleteFromNative", "(Z)V");
    method_db = env->GetMethodID(java_clazz, "callDbFromNative", "(I)V");
    method_pcmtoaac = env->GetMethodID(java_clazz, "encodePcmToAAC", "(I[B)V");

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

void JavaBridge::onCallErrMessage(int type, int errCode, const char *errMessage) {
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

void JavaBridge::onCallLoad(int type, bool isLoad) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(instance, method_load, isLoad);

    } else {
        JNIEnv *jniEnv;
        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(instance, method_load, isLoad);
        vm->DetachCurrentThread();
    }

}

void JavaBridge::onCallComplete(int type, bool isComplete) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(instance, method_complete, isComplete);
    } else {
        JNIEnv *jniEnv;
        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {

        }
        jniEnv->CallVoidMethod(instance, method_complete, isComplete);
        vm->DetachCurrentThread();

    }

}

void JavaBridge::onCallVolumeDb(int type, int db) {
    if (type == MAIN_THREAD) {
        env->CallVoidMethod(instance, method_db, db);

    } else {
        JNIEnv *jniEnv;
        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;
        }
        jniEnv->CallVoidMethod(instance, method_db, db);


        vm->DetachCurrentThread();
    }

}

void JavaBridge::onCallPcmToAAC(int type, int size, void *buffer) {
    if (type == MAIN_THREAD) {

        jbyteArray jbuffer = env->NewByteArray(size);
        env->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));
        env->CallVoidMethod(instance, method_pcmtoaac, size, jbuffer);
        env->DeleteLocalRef(jbuffer);

    } else {
        JNIEnv *jniEnv;

        if (vm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            return;

        }
        jbyteArray jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));
        jniEnv->CallVoidMethod(instance, method_pcmtoaac, size, jbuffer);
        jniEnv->DeleteLocalRef(jbuffer);
        vm->DetachCurrentThread();
    }

}



