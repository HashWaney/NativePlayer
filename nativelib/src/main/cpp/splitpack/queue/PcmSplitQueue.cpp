/**
 * Created by Hash on 2020-04-21.
 */


#include "PcmSplitQueue.h"

PcmSplitQueue::PcmSplitQueue(PlayStatus *playStatus) {
    this->playStatus = playStatus;

    pthread_mutex_init(&mutexBuffer, NULL);
    pthread_cond_init(&condBuffer, NULL);

}

PcmSplitQueue::~PcmSplitQueue() {
    playStatus = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    LOG_D("pCmSplitQueue free")

}

int PcmSplitQueue::getPcmBuffer(PcmSplitEntity **pcmSplitEntity) {
    pthread_mutex_lock(&mutexBuffer);
    while (playStatus != NULL && !playStatus->exit) {
        if (splitQueue.size() > 0) {
            //出队
            *pcmSplitEntity = splitQueue.front();
            splitQueue.pop_front();
            break;
        } else {
            if (!playStatus->exit) {
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;

}

int PcmSplitQueue::clearBuffer() {
    pthread_cond_signal(&condBuffer);

    pthread_mutex_lock(&mutexBuffer);

    while (!splitQueue.empty()) {
        PcmSplitEntity *pcmSplitEntity = splitQueue.front();
        splitQueue.pop_front();
        delete (pcmSplitEntity);
    }

    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int PcmSplitQueue::putPcmBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);

    PcmSplitEntity *pcmSplitEntity = new PcmSplitEntity(buffer, size);
    //TODO push_back
    splitQueue.push_back(pcmSplitEntity);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);


    return 0;
}

void PcmSplitQueue::release() {
    notifyThread();
    clearBuffer();
    LOG_D("release pcm queue success");

}

int PcmSplitQueue::getPcmBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);

    size = splitQueue.size();

    pthread_mutex_unlock(&mutexBuffer);

    return size;
}

int PcmSplitQueue::notifyThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}
