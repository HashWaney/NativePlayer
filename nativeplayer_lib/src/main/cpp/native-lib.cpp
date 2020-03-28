#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_android_media_nativeplayerlib_TestJNI_stringFromJNI(JNIEnv *env, jobject instance) {

    // TODO
    std::string hello = "Hello I am from nativeplayerlib's C++ function to say hello to app module";

    return env->NewStringUTF(hello.c_str());
}