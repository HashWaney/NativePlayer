/**
 * Created by Hash on 2020-04-14.
 */

#include "BufferQueue.h"

BufferQueue::BufferQueue(PlayStatus *status) {
    this->playStatus = status;

    //init mutex
    pthread_mutex_init(&mutexPacket, NULL);
    //init cond
    pthread_cond_init(&condPacket, NULL);

}

BufferQueue::~BufferQueue() {

}

int BufferQueue::putPacketToQueue(AVPacket *packet) {

    pthread_mutex_lock(&mutexPacket);

    queuePacket.push(packet);

//    LOG_E("放入一个AVPacket 到容器中 ，队列个数为 %d", queuePacket.size());
    //todo 通知消费者进行消费
    pthread_cond_signal(&condPacket);

    pthread_mutex_unlock(&mutexPacket);


    return 0;
}

int BufferQueue::getPacketFromQueue(AVPacket *packet) {
    pthread_mutex_lock(&mutexPacket);
    while (playStatus != NULL && !playStatus->exit) {

        if (queuePacket.size() > 0) {
            //从队列中的头部取出一个AVPacket 顺序存储的。 因此解码和读取应该是同步的，
            AVPacket *avPacket = queuePacket.front();
            //将取出的AVPacket TODO
            if (av_packet_ref(packet, avPacket) == 0) {
                queuePacket.pop();
            }
            //释放
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
//            LOG_D("取出一个AvPacket 还剩下%d个", queuePacket.size());
            //TODO 跳出循环，每次只取出一个
            break;
        } else {
            //TODO 等待队列中数据填充
            pthread_cond_wait(&condPacket, &mutexPacket);
        }

    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int BufferQueue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

void BufferQueue::clearQueue() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty()) {
        AVPacket *avPacket = queuePacket.front();
        //pop
        queuePacket.pop();
        //free avPacket
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;

    }
    pthread_mutex_unlock(&mutexPacket);

}
