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
JNIEXPORT jstring JNICALL
Java_cn_hash_mm_nativelib_Demo_stringFromJNI(JNIEnv *env, jobject jobject) {
    char *str = "Hello I am from C++";
    return env->NewStringUTF(str);
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1prepare(JNIEnv *env, jobject instance, jstring source) {
    //将jstring 转换为const char* 是因为 ffmpeg 打开url的时候传入const char* avformat_open_input();
    const char *url = env->GetStringUTFChars(source, 0);


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

    if (fFmpegController != NULL) {
        fFmpegController->startPlay();
    }

}