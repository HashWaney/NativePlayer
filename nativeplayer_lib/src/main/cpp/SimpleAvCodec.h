//
// Created by 王庆 on 2020-03-30.
//

#ifndef PLAYER_SIMPLEAVCODEC_H
#define PLAYER_SIMPLEAVCODEC_H

extern "C"
{
#include "include/libavformat/avformat.h"
#include "include/libavcodec/avcodec.h"
};

#include "CallJavaBridge.h"

#include "pthread.h"
#include "AndroidLog.h"

#include "AVPacketQueue.h"
#include "AVPlayStatus.h"


class SimpleAvCodec {
public:
    const char *_url;
    AVCodecParameters *avCodecParameters = NULL;
    AVFormatContext *avCodecContext = NULL;
    int streamIndex = -1;
    CallJavaBridge *callJavaBridge = NULL;
//
    AVPacketQueue *avPacketQueue = NULL;
    AVPlayStatus *avPlayStatus = NULL;




public:
    SimpleAvCodec(AVPlayStatus* avPlayStatus,const char *_url, CallJavaBridge *callJavaBridge);

    ~SimpleAvCodec();

    void prepared();

    void decodePrepared();

    void startDecode();
};


#endif //PLAYER_SIMPLEAVCODEC_H
