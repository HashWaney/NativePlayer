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
#include "../javabridge/JavaBridge.h"
#include "../status/PlayStatus.h"
#include "time.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/time.h"
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
    FFmpegController(JavaBridge *javaBridge, const char *url);

    ~FFmpegController();

    void prepare(const char *url);

    void prepareTask();

    void startPlay();

    void pausePlay();

    void resumePlay();

    void release();

    void seek(uint64_t second);

    void setVolume(int volume);


};


#endif //AUDIOPLAYER_FFMPEGCONTROLLER_H
