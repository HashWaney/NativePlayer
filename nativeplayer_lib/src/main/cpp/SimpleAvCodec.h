//
// Created by 王庆 on 2020-03-30.
//

#ifndef PLAYER_SIMPLEAVCODEC_H
#define PLAYER_SIMPLEAVCODEC_H

extern "C"
{
#include "include/libavformat/avformat.h"
#include "include/libavcodec/avcodec.h"
#include "include/libswresample/swresample.h"
};

#include "CallJavaBridge.h"

#include "pthread.h"
#include "AndroidLog.h"

#include "AVPacketQueue.h"
#include "AVPlayStatus.h"

#define SAMPLE_BUFFER_SIZE (44100*2*2)

class SimpleAvCodec {
public:
    const char *_url;
    AVCodecParameters *avCodecParameters = NULL;
    AVCodecContext *avCodecContext = NULL;
    AVFormatContext *avFormatContext = NULL;
    int streamIndex = -1;
    CallJavaBridge *callJavaBridge = NULL;
    AVPacketQueue *avPacketQueue = NULL;
    AVPlayStatus *avPlayStatus = NULL;

    //重采样线程
    pthread_t play_thread;


    int mallocResult = 0;
    AVFrame *avFrame = NULL;
    AVPacket *avPacket = NULL;

    uint8_t *buffer = NULL;

    int data_size = 0;


public:
    SimpleAvCodec(AVPlayStatus *avPlayStatus, const char *_url, CallJavaBridge *callJavaBridge);

    ~SimpleAvCodec();

    void prepared();

    void preparedDecode();

    void startDecode();

    void playAudio();

    int resample();

    void releaseResource();
};


#endif //PLAYER_SIMPLEAVCODEC_H
