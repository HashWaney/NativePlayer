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

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

class FFmpegController {

public:
    const char *url = NULL;
    pthread_t prepareThread;
    AVFormatContext *avFormatContext = NULL;

    AudioController *audioController = NULL;


    ///////JavaBridge////////
    JavaBridge *javaBridge = NULL;

    ////playStatus////////
    PlayStatus *playStatus = NULL;


public:
    FFmpegController(JavaBridge *javaBridge);

    ~FFmpegController();

    void prepare(const char *url);

    void prepareTask();

    void startPlay();


};


#endif //AUDIOPLAYER_FFMPEGCONTROLLER_H
