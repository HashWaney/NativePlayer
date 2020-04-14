/**
 * Created by Hash on 2020-04-14.
 */


#ifndef AUDIOPLAYER_BUFFERQUEUE_H
#define AUDIOPLAYER_BUFFERQUEUE_H

#include "queue"
#include "pthread.h"
#include "../log/AudioLog.h"
#include "../status/PlayStatus.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};


class BufferQueue {
public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    PlayStatus *playStatus = NULL;


public:
    BufferQueue(PlayStatus* playStatus);

    ~BufferQueue();

    int putPacketToQueue(AVPacket *packet);

    int getPacketFromQueue(AVPacket *packet);

    int getQueueSize();


};


#endif //AUDIOPLAYER_BUFFERQUEUE_H
