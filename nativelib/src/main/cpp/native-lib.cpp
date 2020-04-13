/**
 * Created by Hash on 2020-04-13.
 */

#include<jni.h>




extern "C"
JNIEXPORT jstring JNICALL
Java_cn_hash_mm_nativelib_Demo_stringFromJNI(JNIEnv *env, jobject jobject) {
    char *str = "Hello I am from C++";
    return env->NewStringUTF(str);
}