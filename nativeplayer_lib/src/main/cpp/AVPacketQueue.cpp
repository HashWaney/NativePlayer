//
// Created by 王庆 on 2020-03-31.
//

#include "AVPacketQueue.h"


AVPacketQueue::~AVPacketQueue() {

//    pthread_cond_destroy(&condPacket);
//    pthread_mutex_destroy(&mutexPacket);
}

//接收一个AVPlayStatus状态 用来控制当前缓存的读取
AVPacketQueue::AVPacketQueue(AVPlayStatus *avPlayStatus) {
    this->avPlayStatus = avPlayStatus;

    pthread_cond_init(&condPacket, NULL);

    pthread_mutex_init(&mutexPacket, NULL);


}

void AVPacketQueue::putAvPacket(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPacket);
    avPacketQueue.push(avPacket);
//    LOGD("存入一个AvPacket 到队列中，个数为：%d", avPacketQueue.size());
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);


}

//保证引用是存在的。因为你去取队列中的数据，AvPacket是指向data引用的，不存储真实的内存
int AVPacketQueue::getAvPacket(AVPacket *packet) {

//    pthread_mutex_lock(&mutexPacket);
//
//    while (avPlayStatus != NULL && !avPlayStatus->exit) {
//        // TODO 从队列中取数据
//        if (avPacketQueue.size() > 0) {
//            //从队列中的表头开始取 先进先出
//            AVPacket *packetFromQueue = avPacketQueue.front();
//            //引用赋值成功
//            if (av_packet_ref(avPacket, packetFromQueue) == 0) {
//                //出队列操作
//                avPacketQueue.pop();
//
//            }
//            av_packet_free(&packetFromQueue);
//            av_free(packetFromQueue);
//            packetFromQueue = NULL;
//            LOGD("从队列中取出一个AvPacket ，还剩下%d个", avPacketQueue.size());
//
//            break;
//
//        } else {
//            //TODO 等待生产者生产
//            pthread_cond_wait(&condPacket, &mutexPacket);
//        }
//    }
//
//    pthread_mutex_unlock(&mutexPacket);
    pthread_mutex_lock(&mutexPacket);
    while (avPlayStatus != NULL && !avPlayStatus->exit) {
        if (avPacketQueue.size() > 0) {
            AVPacket *avPacket = avPacketQueue.front();
            if (av_packet_ref(packet, avPacket) == 0) { //成功
                avPacketQueue.pop();
            }else{ //失败
                return -1;
            }
            //TODO 此处重复释放内存。导致程序崩溃。调用该方法的地方已经进行内存的释放，不用重复释放。
//            av_packet_free(&avPacket);
//            av_free(avPacket);
//            avPacket = NULL;
//            LOGD("从队列里面取出一个AVpacket，还剩下 %d 个", avPacketQueue.size());
            break;
        } else {
            pthread_cond_wait(&condPacket, &mutexPacket);
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;

}

int AVPacketQueue::getAVPacketQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = avPacketQueue.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}
