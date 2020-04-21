/**
 * Created by Hash on 2020-04-21.
 */


#ifndef AUDIOPLAYER_PCMSPLITQUEUE_H
#define AUDIOPLAYER_PCMSPLITQUEUE_H

#include <deque>
#include "SoundTouch.h"
#include "../entity/PcmSplitEntity.h"
#include "../status/PlayStatus.h"
#include "pthread.h"
#include "../log/AudioLog.h"

using namespace soundtouch;

class PcmSplitQueue {

public:
    //TODO deque 出队
    std::deque<PcmSplitEntity *> splitQueue;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    PlayStatus *playStatus = NULL;


public:
    PcmSplitQueue(PlayStatus *playStatus);

    ~PcmSplitQueue();

    int getPcmBuffer(PcmSplitEntity **pcmSplitEntity);

    int clearBuffer();

    int putPcmBuffer(SAMPLETYPE *buffer, int size);

    void release();

    int getPcmBufferSize();

    int notifyThread();

};


#endif //AUDIOPLAYER_PCMSPLITQUEUE_H
