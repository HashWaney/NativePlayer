/**
 * Created by Hash on 2020-04-13.
 */

#include<jni.h>
#include "log/AudioLog.h"
#include "ffmpeg/FFmpegController.h"

FFmpegController *fFmpegController = NULL;


extern "C"
JNIEXPORT jstring JNICALL
Java_cn_hash_mm_nativelib_Demo_stringFromJNI(JNIEnv *env, jobject instance) {
    char *str = "Hello I am from C++";
    return env->NewStringUTF(str);
}

extern "C"
JNIEXPORT void JNICALL
Java_cn_hash_mm_nativelib_PlayController_n_1prepare(JNIEnv *env, jobject instance, jstring source) {
    //将jstring 转换为const char* 是因为 ffmpeg 打开url的时候传入const char* avformat_open_input();
    const char *url = env->GetStringUTFChars(source, 0);


    if (fFmpegController == NULL) {
        fFmpegController = new FFmpegController();
    }
    fFmpegController->prepare(url);
}