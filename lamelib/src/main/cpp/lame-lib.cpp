/**
 * Created by Hash on 2020-04-18.
 */

#include "jni.h"
#include "libmp3lame/lame.h"

extern "C"
JNIEXPORT jstring
JNICALL Java_cn_hash_mm_lamelib_LameHelper_getVersion(JNIEnv *env, jobject instance) {


    return env->NewStringUTF(get_lame_version());
}
