/**
 * Created by Hash on 2020-04-13.
 */


#ifndef AUDIOPLAYER_FFMPEGCONTROLLER_H
#define AUDIOPLAYER_FFMPEGCONTROLLER_H


#include <cstddef>
#include <sys/types.h>
#include "pthread.h"
#include "../log/AudioLog.h"
#include "../audio/AudioController.h"
#include "../bridge/JavaBridge.h"
#include "../status/PlayStatus.h"


extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/time.h>
};

class FFmpegController {

public:
    const char *url = NULL;
    pthread_t prepareThread;
    AVFormatContext *avFormatContext = NULL;

    AudioController *audioController = NULL;

    bool isDecodeTerminate = false;


    ///////JavaBridge////////
    JavaBridge *javaBridge = NULL;

    ////playStatus////////
    PlayStatus *playStatus = NULL;

    //total time
    int duration = 0;

    //release lock
    pthread_mutex_t decode_lock;

    //lock seek
    pthread_mutex_t seek_lock;


public:
    FFmpegController(PlayStatus *playStatus, JavaBridge *javaBridge, const char *url);

    ~FFmpegController();

    void prepare();

    void prepareTask();

    void startPlay();

    void pausePlay();

    void resumePlay();

    void release();

    void seek(uint64_t second);

    void setVolume(int volume);

    void setMute(int muteType);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSampleRate();

    void startRecord(bool record);

    void pauseRecord(bool record);

    void resumeRecord(bool record);

    void stopRecord(bool record);


};


#endif //AUDIOPLAYER_FFMPEGCONTROLLER_H
