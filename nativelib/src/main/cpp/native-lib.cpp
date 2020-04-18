/**
 * Created by Hash on 2020-04-13.
 */

#include<jni.h>
#include "log/AudioLog.h"
#include "ffmpeg/FFmpegController.h"
#include "javabridge/JavaBridge.h"
#include "lame.h"

JavaBridge *javaBridge = NULL;

FFmpegController *fFmpegController = NULL;

_JavaVM *javaVM = NULL;

int isExit = 0;

pthread_t thread_StartPlay;

void *startToPlay(void *data) {
    FFmpegController *fFmpegController = (FFmpegController *) data;
    fFmpegController->startPlay();


    pthread_exit(&thread_StartPlay);
}



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

/**
 * 初始化只能初始化一次。不能反复初始化。
 */
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
        javaBridge->onCallLoad(MAIN_THREAD, true);
        fFmpegController = new FFmpegController(javaBridge, url);
        //函数只能调用一次啊 不能反复调用啊 TODO ? why
        fFmpegController->prepare(url);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1startPlay(JNIEnv *env, jobject instance) {

    LOG_D("startPlay");
    if (fFmpegController != NULL) {
        pthread_create(&thread_StartPlay, NULL, startToPlay, fFmpegController);
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
Java_cn_hash_mm_nativelib_PlayController_n_1stop(JNIEnv *env, jobject instance, jint nextPage) {
    LOG_E("current exit %d", isExit);
    if (isExit != 0) {
        return;
    }
    isExit = 1;
    jclass j_clazz = env->GetObjectClass(instance);
    jmethodID method_next = env->GetMethodID(j_clazz, "callNextAfterInvokeN_Stop", "()V");
    if (fFmpegController != NULL) {

        fFmpegController->release();

        delete (fFmpegController);
        fFmpegController = NULL;

        if (javaBridge != NULL) {
            delete (javaBridge);
            javaBridge = NULL;
        }
    }
    isExit = 0;
    //TODO 说明是播放下一首。
    if (nextPage != -1) {
        env->CallVoidMethod(instance, method_next);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1seek(JNIEnv *env, jobject instance, jint seconds) {

    if (fFmpegController != NULL) {
        fFmpegController->seek(seconds);
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1duration(JNIEnv *env, jobject instance) {
    if (fFmpegController != NULL) {
        return fFmpegController->duration;
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1setvolume(JNIEnv *env, jobject instance, jint volume) {

    if (fFmpegController != NULL) {
        fFmpegController->setVolume(volume);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1muteType(JNIEnv *env, jobject instance, jint muteType) {

    if (fFmpegController != NULL) {
        fFmpegController->setMute(muteType);
    }

}extern "C"
JNIEXPORT jstring JNICALL
Java_cn_hash_mm_nativelib_PlayController_getLameVersion(JNIEnv *env, jobject instance) {

    // TODO


    return env->NewStringUTF(get_lame_version());
}