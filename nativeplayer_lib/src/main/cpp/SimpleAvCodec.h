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

#include "pthread.h"
#include "AndroidLog.h"

class SimpleAvCodec {
public:
    const char *_url;
    AVCodecParameters *avCodecParameters = NULL;
    AVFormatContext *avCodecContext = NULL;
    int streamIndex = -1;


public:
    SimpleAvCodec(const char *_url);

    ~SimpleAvCodec();

    void prepared();

    void decodePrepared();
};


#endif //PLAYER_SIMPLEAVCODEC_H
