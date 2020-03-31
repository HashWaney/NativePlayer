//
// Created by 王庆 on 2020-03-31.
//

#ifndef PLAYER_AVPACKETQUEUE_H
#define PLAYER_AVPACKETQUEUE_H

#include "pthread.h"
#include "queue"
#include "AVPlayStatus.h"

extern "C"
{
#include "include/libavcodec/avcodec.h"
};

#include "AndroidLog.h"

class AVPacketQueue {

public:
    pthread_mutex_t mutexPacket; //互斥锁
    pthread_cond_t condPacket;//条件锁
    std::queue<AVPacket *> avPacketQueue; //将AVPacket读取到队列中进行缓存
    AVPlayStatus *avPlayStatus = NULL;

public:
    AVPacketQueue(AVPlayStatus *avPlayStatus);


    ~AVPacketQueue();

    void putAvPacket(AVPacket *avPacket);

    void getAvPacket(AVPacket *avPacket);

    int getAVPacketQueueSize();


};


#endif //PLAYER_AVPACKETQUEUE_H
