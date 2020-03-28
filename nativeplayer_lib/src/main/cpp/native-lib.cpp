#include <jni.h>
#include <string>
#include "android/log.h"

extern "C" {
  #include "include/libavformat/avformat.h"
}
//##__VA_ARGS__ 宏前面加上##的作用在于，当可变参数的个数为0时，这里的##起到把前面多余的","去掉的作用,否则会编译出错
//__VA_ARGS__ 是一个可变参数的宏，很少人知道这个宏，这个可变参数的宏是新的C99规范中新增的，目前似乎只有gcc支持（VC6.0的编译器不支持）。
//实现思想就是宏定义中参数列表的最后一个参数为省略号（也就是三个点）。
#define LOGD(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG,"Hash",FORMAT,##__VA_ARGS__);


extern "C"
JNIEXPORT jstring JNICALL

Java_com_android_media_nativeplayerlib_TestJNI_stringFromJNI(JNIEnv *env, jobject instance) {

    // TODO
    std::string hello = "Hello I am from nativeplayerlib's C++ function to say hello to app module";

    AVCodec *avCodec = av_codec_next(NULL);

//    while (avCodec != NULL) {
//        switch (avCodec->type) {
//            case AVMEDIA_TYPE_AUDIO:
//
//                LOGD("type is audio name :%s", avCodec->name);
//
//                break;
//            case AVMEDIA_TYPE_VIDEO:
//                LOGD("type is video name :%s", avCodec->name);
//                break;
//            default:
//                LOGD("type is unkonw  name :%s", avCodec->name);
//                break;
//        }
//    }


    return env->NewStringUTF(hello.c_str());
}
