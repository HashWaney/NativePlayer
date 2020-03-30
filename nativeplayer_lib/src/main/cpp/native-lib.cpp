#include <jni.h>
#include <string>
#include "SimpleAvCodec.h"
#include "AndroidLog.h"

extern "C" {
#include "include/libavformat/avformat.h"
}
//##__VA_ARGS__ 宏前面加上##的作用在于，当可变参数的个数为0时，这里的##起到把前面多余的","去掉的作用,否则会编译出错
//__VA_ARGS__ 是一个可变参数的宏，很少人知道这个宏，这个可变参数的宏是新的C99规范中新增的，目前似乎只有gcc支持（VC6.0的编译器不支持）。
//实现思想就是宏定义中参数列表的最后一个参数为省略号（也就是三个点）。



extern "C"
JNIEXPORT jstring JNICALL

Java_com_android_media_nativeplayerlib_TestJNI_stringFromJNI(JNIEnv *env, jobject instance) {

    // TODO
    std::string hello = "Hello I am from nativeplayerlib's C++ function to say hello to app module";

    LOGD("hello i am test for log ");


    return env->NewStringUTF(hello.c_str());
}


SimpleAvCodec *avCodec = NULL;
CallJavaBridge *callJavaBridge = NULL;
_JavaVM *javaVM = NULL;


extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_nativeplayerlib_AudioPlayer_prepared(JNIEnv *env, jobject instance,
                                                            jstring url_) {
    const char *url = env->GetStringUTFChars(url_, 0);

    if (avCodec == NULL) {
        if (callJavaBridge == NULL) {
            callJavaBridge = new CallJavaBridge(javaVM, env, &instance);

        }
        LOGD("avCodec init ")
        avCodec = new SimpleAvCodec(url, callJavaBridge);
        LOGD("avCodec prepared start")
        avCodec->prepared();
        LOGD("avCodec prepared end")

    }

//    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT
jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {

    JNIEnv *env;
    javaVM = vm;


    // & 就是取env的指针  env 本身就是一个指针 然后通过取地址符 就相当于取了env指针的指针地址。

    jint i = vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (i != JNI_OK) {
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;


}

/**
 * 该方法是用来进行解码操作
 */

extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_nativeplayerlib_AudioPlayer_startDecode(JNIEnv *env, jobject instance) {
    if (avCodec != NULL) {
        avCodec->startDecode();
    }

}