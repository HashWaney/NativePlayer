#include <jni.h>
#include <string>

#include "pthread.h"

#include "AndroidLog.h"


//pthread_t *pNormal;
// 这种写法声明一个pthread_t 指针 容易报 signal SIGSEGV 执行一个无效的内存引用，
// 因此只需要定义一个pthread_t 就好了，然后通过取地址符号（&pNormal)获取一个指针对象。

pthread_t  pNormal;

void* normalCallback(void *data) {
    LOGD("enter normal thread callback");
    //退出线程 防止内存泄漏
    pthread_exit(&pNormal);

}


extern "C"

JNIEXPORT void JNICALL
Java_com_android_media_pthreaddemo_ThreadHandler_normalCreateThread(JNIEnv *env, jobject instance) {
    LOGD("c++ create pthread");
    pthread_create(&pNormal, NULL, normalCallback, NULL);

    //退出线程 防止内存泄漏 该句代码最好在线程回调执行完之后在退出线程，回调有可能是异步的，因此这里直接就退出线程可能会有问题
//    pthread_exit(pNormal);


}


extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_pthreaddemo_ThreadHandler_mutexThread(JNIEnv *env, jobject instance) {

    // TODO
    LOGD("c++ mutexThread")

}


extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_pthreaddemo_ThreadHandler_callbackFromCplus(JNIEnv *env, jobject instance) {

    LOGD("c++ callback ")

}