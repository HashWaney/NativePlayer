/**
 * Created by Hash on 2020-04-13.
 */

#include<jni.h>
#include "log/AudioLog.h"
#include "ffmpeg/FFmpegController.h"
#include "javabridge/JavaBridge.h"


JavaBridge *javaBridge = NULL;

FFmpegController *fFmpegController = NULL;

_JavaVM *javaVM = NULL;



//TODO 获取JavaVM 必须重写Java_OnLoad

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *versio) {
    JNIEnv *env;
    javaVM = vm;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}


extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1prepare(JNIEnv *env, jobject instance, jstring source) {
    //将jstring 转换为const char* 是因为 ffmpeg 打开url的时候传入const char* avformat_open_input();
    const char *url = env->GetStringUTFChars(source, 0);
    LOG_D("prepare decode :%s", url);
    if (fFmpegController == NULL) {
        if (javaBridge == NULL) {
            javaBridge = new JavaBridge(javaVM, env, &instance);
        }
        fFmpegController = new FFmpegController(javaBridge);
    }
    fFmpegController->prepare(url);
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1startPlay(JNIEnv *env, jobject instance) {

    LOG_D("startPlay");
    if (fFmpegController != NULL) {
        fFmpegController->startPlay();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1pause(JNIEnv *env, jobject instance) {
    LOG_D("pause play");

    if (fFmpegController != NULL) {
        fFmpegController->pausePlay();
    }


}extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1resume(JNIEnv *env, jobject instance) {
    LOG_D("resume play");
    if (fFmpegController != NULL) {
        fFmpegController->resumePlay();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1stop(JNIEnv *env, jobject instance) {
    if (fFmpegController != NULL) {

        fFmpegController->release();

        delete (fFmpegController);
        fFmpegController = NULL;

        if (javaBridge != NULL) {
            delete (javaBridge);
            javaBridge = NULL;
        }

    }

}extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1seek(JNIEnv *env, jobject instance, jint seconds) {

    if (fFmpegController != NULL) {
        fFmpegController->seek(seconds);
    }

}