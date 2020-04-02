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

#include "AndroidLog.h"
#include "CallJavaBridge.h"

#include "pthread.h"


#include "AVPacketQueue.h"
#include "AVPlayStatus.h"

#include "SLES/OpenSLES_Android.h"
#include "SLES/OpenSLES.h"

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

    uint8_t *out_buffer = NULL;

    int data_size = 0;


    //引擎对象 混音器对象
    SLObjectItf engineObj, outputMixObj, pcmPlayerObj;
    //引擎接口
    SLEngineItf engineEngine;

    //环境混音器接口
    SLEnvironmentalReverbItf outputMixItf;
    SLEnvironmentalReverbSettings environmentalReverbSettings;

    //播放器接口
    SLPlayItf playItf;

    //缓冲区接口
    SLAndroidSimpleBufferQueueItf androidSimpleBufferQueueItf;

    //声音接口
    SLVolumeItf volumeItf;


public:
    SimpleAvCodec(AVPlayStatus *avPlayStatus, const char *_url, CallJavaBridge *callJavaBridge);

    ~SimpleAvCodec();

    void prepared();

    void preparedDecode();

    void startDecode();

    void playAudio();

    int resample();

    void initOpenSlEs();

    void releaseResource();
};


#endif //PLAYER_SIMPLEAVCODEC_H
