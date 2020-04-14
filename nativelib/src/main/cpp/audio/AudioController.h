/**
 * Created by Hash on 2020-04-13.
 */


#ifndef AUDIOPLAYER_AUDIOCONTROLLER_H
#define AUDIOPLAYER_AUDIOCONTROLLER_H
extern "C"
{
#include "libavcodec/avcodec.h"
};


class AudioController {
public:
    int streamIndex = -1; //遍历数据流的为音频流的角标
    AVCodecContext *avCodecContext = NULL; //解码器上下文
    AVCodecParameters *avCodecParameters = NULL;//解码器参数
    int sample_rate = 0;//采样率

    AVRational timeBase;

public:
    AudioController(int sample_rate);

    ~AudioController();

};


#endif //AUDIOPLAYER_AUDIOCONTROLLER_H
