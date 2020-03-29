#include <jni.h>
#include <string>

#include "pthread.h"
#include "unistd.h"
#include "AndroidLog.h"

#define CONSUMERS_COUNT 2
#define PRODUCERS_COUNT 2

pthread_mutex_t g_mutex; //互斥锁

pthread_cond_t g_cond; //条件变量


pthread_t g_thread[CONSUMERS_COUNT + PRODUCERS_COUNT];

int share_variable = 0;


void *consumer(void *args) {

    long num = (long) (args);
    while (1) {
        //1。互斥锁
        LOGD("consumer %d lock\n", num);
        pthread_mutex_lock(&g_mutex);

        while (share_variable == 0) {
            LOGD("consumer %d begin wait a condition\n", num);
            pthread_cond_wait(&g_cond, &g_mutex);
        }


        LOGD("consumer %d end wait a condition\n", num);
        LOGD("consumer %d restart consume product\n", num);
        //资源减少
        --share_variable;
        //释放互斥锁
        LOGD("consumer %d free lock\n", num);
        pthread_mutex_unlock(&g_mutex);

        sleep(1);

    }

}


void *producer(void *arg) {

    long num = (long) (arg);
    while (1) {

        LOGD("producer %d lock\n", num);
        pthread_mutex_lock(&g_mutex);

        LOGD("producer %d begin produce product\n", num);
        ++share_variable;
        LOGD("producer %d end   produce product\n", num);

        pthread_cond_signal(&g_cond);

        LOGD("producer %d notified consumer by condition \n", num);

        LOGD("producer %d free lock \n", num);
        pthread_mutex_unlock(&g_mutex);

        sleep(5);


    }


}







//pthread_t *pNormal;
// 这种写法声明一个pthread_t 指针 容易报 signal SIGSEGV 执行一个无效的内存引用，
// 因此只需要定义一个pthread_t 就好了，然后通过取地址符号（&pNormal)获取一个指针对象。

pthread_t pNormal;

void *normalCallback(void *data) {
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



    //1。生产者

//    pthread_cond_init()
//
//    pthread_cond_destroy()
//
//    pthread_cond_wait()
//
//    pthread_cond_signal()
//
//    pthread_cond_broadcast()

    //2。消费者

    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);

    for (int i = 0; i < CONSUMERS_COUNT; ++i) {
        pthread_create(&g_thread[i], NULL, consumer,
                       (void *) i);

    }
    sleep(1);

    for (int i = 0; i < PRODUCERS_COUNT; ++i) {
        pthread_create(&g_thread[i], NULL, producer, (void *) i);
    }

    for (int i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT; ++i) {
        pthread_join(g_thread[i], NULL);
    }

    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_cond);

}


extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_pthreaddemo_ThreadHandler_callbackFromCplus(JNIEnv *env, jobject instance) {

    LOGD("c++ callback ")

}