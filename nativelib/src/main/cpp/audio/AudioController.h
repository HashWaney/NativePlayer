/**
 * Created by Hash on 2020-04-13.
 */


#ifndef AUDIOPLAYER_AUDIOCONTROLLER_H
#define AUDIOPLAYER_AUDIOCONTROLLER_H
extern "C"
{
#include "libavcodec/avcodec.h"
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include "libswresample/swresample.h"
};

#include "../queue/BufferQueue.h"
#include "../status/PlayStatus.h"
#include "../javabridge/JavaBridge.h"


class AudioController {
public:
    int streamIndex = -1; //遍历数据流的为音频流的角标
    AVCodecContext *avCodecContext = NULL; //解码器上下文
    AVCodecParameters *avCodecParameters = NULL;//解码器参数
    JavaBridge *javaBridge = NULL;

    ////////////play thread///////////////
    pthread_t playThread;


    ////////BufferQueue.cpp//////
    BufferQueue *bufferQueue = NULL;

    PlayStatus *playStatus = NULL;


    ////////////OpenSLES//////////////////////
    SLObjectItf engineObj = NULL;
    SLEngineItf engineItf = NULL;

    SLObjectItf outputMixObj = NULL;
    SLEnvironmentalReverbItf outPutMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


    SLObjectItf playObj = NULL;
    SLPlayItf playItf = NULL;

    SLAndroidSimpleBufferQueueItf androidSimpleBufferQueueItf = NULL;


    //////////////resample AVPacket to AVFrame///////////////////
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int codecOperateFlag = 0;
    uint8_t *receiveDataFromFrameBuffer;
    int data_size = 0;


    //////////////////time/////////////////////////////
    int duration = 0;
    AVRational timeBase;
    int sample_rate = 0;//采样率
    double clock; //播放的时长
    double now_time;//当前调用时间
    double last_time; //上一次调用时间


public:
    AudioController(JavaBridge *javaBridge, PlayStatus *playStatus, int sample_rate);

    ~AudioController();

    void playMusic();

    void initOpenSLES();

    SLuint32 getCurrentSampleRate(int sample_rate);

    int resampleAudio();


    void pause();

    void resume();

    void release();

};


#endif //AUDIOPLAYER_AUDIOCONTROLLER_H
