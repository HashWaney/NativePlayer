/**
 * Created by Hash on 2020-04-13.
 */


#ifndef AUDIOPLAYER_AUDIOCONTROLLER_H
#define AUDIOPLAYER_AUDIOCONTROLLER_H

#include "SoundTouch.h"
#include "../queue/BufferQueue.h"
#include "../status/PlayStatus.h"
#include "../bridge/JavaBridge.h"
#include "../splitpack/queue/PcmSplitQueue.h"
#include "../splitpack/entity/PcmSplitEntity.h"

using namespace soundtouch;

extern "C"
{
#include "libavcodec/avcodec.h"
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include "libswresample/swresample.h"
};

class AudioController {
public:
    int streamIndex = -1; //遍历数据流的为音频流的角标
    AVCodecContext *avCodecContext = NULL; //解码器上下文
    AVCodecParameters *avCodecParameters = NULL;//解码器参数
    ////////BufferQueue.cpp//////
    BufferQueue *bufferQueue = NULL;

    /////////PlayStatus///////
    PlayStatus *playStatus = NULL;


    /////JavaBridge///////
    JavaBridge *javaBridge = NULL;

    ////////////play thread///////////////
    pthread_t playThread;


    ////////////OpenSLES//////////////////////
    SLObjectItf engineObj = NULL; //引擎接口
    SLEngineItf engineItf = NULL;

    SLObjectItf outputMixObj = NULL; //混音器
    SLEnvironmentalReverbItf outPutMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;


    SLObjectItf playObj = NULL; //播放器接口
    SLPlayItf playItf = NULL;

    // 缓存队列接口
    SLAndroidSimpleBufferQueueItf androidSimpleBufferQueueItf = NULL;

    //音量接口
    SLVolumeItf pcmVolumePlay = NULL;

    //声道接口
    SLMuteSoloItf muteSoloItf = NULL;


    //////////////resample AVPacket to AVFrame///////////////////
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int codecOperateFlag = 0;
    uint8_t *receiveDataFromFrameBuffer;
    int sample_rate = 0;//采样率
    int data_size = 0;
    //////////////////time/////////////////////////////
    int duration = 0;
    AVRational timeBase;
    double clock; //播放的时长
    double now_time;//当前调用时间
    double last_time; //上一次调用时间

    int currentVolume = 100;// 当前音量

    int currentMuteType = 2;// 立体声


    SoundTouch *soundTouch = NULL;

    SAMPLETYPE *sound_touch_out_buffer = NULL;

    uint8_t *out_buffer = NULL;

    float pitch = 1.0f;
    float speed = 1.0f;

    bool finish = true;

    int number = 0;
    int soundTouchReceiveNum = 0;

    bool record = false;


    ///////pcm split pack//////////
    pthread_t pcmBufferThread;
    PcmSplitQueue *pcmSplitQueue = NULL;

    int defaultSize = 4096;


public:
    AudioController(JavaBridge *javaBridge, PlayStatus *playStatus, int sample_rate);

    ~AudioController();

    void playMusic();

    void initOpenSLES();

    SLuint32 getCurrentSampleRate(int sample_rate);

    int resampleAudio(void **pcmBuffer);

    void stop();

    void pause();

    void resume();

    void release();

    void setAudioVolume(int volume);

    void setMuteType(int muteType);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getAudioDb(char *pcmData, size_t d);

    void startRecord(bool record);

    void pauseRecord(bool record);

    void resumeRecord(bool record);

    void stopRecord(bool record);

};


#endif //AUDIOPLAYER_AUDIOCONTROLLER_H
