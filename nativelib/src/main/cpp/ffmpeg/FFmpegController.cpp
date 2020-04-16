/**
 * Created by Hash on 2020-04-13.
 */


#include "FFmpegController.h"


FFmpegController::FFmpegController(JavaBridge *javaBridge, const char *url) {
    this->url = url;
    this->javaBridge = javaBridge;
    playStatus = new PlayStatus();
    pthread_mutex_init(&decode_lock, NULL);
    pthread_mutex_init(&seek_lock, NULL);


}

FFmpegController::~FFmpegController() {

    pthread_mutex_destroy(&decode_lock);
    pthread_mutex_destroy((&seek_lock));


}

void *prepareCallBack(void *data) {
    FFmpegController *controller = (FFmpegController *) data;
    controller->prepareTask();
    pthread_exit(&controller->prepareThread);

}


void FFmpegController::prepare(const char *url) {
    //TODO 在C++里面开启一个子线程读取url信息
    this->url = url;
    pthread_create(&prepareThread, NULL, prepareCallBack, this);


}

int avformat_callback(void *data) {
    FFmpegController *fFmpegController = (FFmpegController *) data;
    if (fFmpegController->playStatus->exit) {
        return AVERROR_EOF;
    }
    return 0;

}

//2.准备工作（解码操作）
void FFmpegController::prepareTask() {

    pthread_mutex_lock(&decode_lock);
    LOG_E("prepare to decode  form native");

    //1.注册所有的编解码器 在libavformat/avformat.h
    av_register_all();


    //2.网络初始化
    avformat_network_init();

    //3.分配avFormatContext 该上下文包含了流媒体的信息：比如是音频流 编解码 视频流
    avFormatContext = avformat_alloc_context();
    LOG_E("avFormatContext  aaaaa   ");

    //TODO
//    avFormatContext->interrupt_callback.callback = avformat_callback;
//    avFormatContext->interrupt_callback.opaque = this;

    //4.打开url地址 读取url信息 &avFormatContext 这个是引用  也就是指向指针的指针
    int errorCode = avformat_open_input(&avFormatContext, url, NULL, NULL);
    LOG_E("error Code: %d", errorCode);
    char buffer[1024] = {0};
    if (errorCode != 0) {
        if (javaBridge != NULL) {
            javaBridge->onCallErrMessage(TASK_THREAD, 1001, "can not open error url");
        }
        LOG_E("can not open error url: %s", url);
        av_strerror(errorCode, buffer, sizeof(buffer));
        LOG_D("error message: %s \n error code :%d", buffer, errorCode);
        isDecodeTerminate = true;
        pthread_mutex_unlock(&decode_lock);
        return;
    }

    LOG_D("open input success");

    //5.判断读取的媒体信息是否包含流信息
    if (avformat_find_stream_info(avFormatContext, NULL)) {
        LOG_E("can not find streams from %s", url);
        if (javaBridge != NULL) {
            javaBridge->onCallErrMessage(TASK_THREAD, 1002, "can not find streams from url");;
        }
        isDecodeTerminate = true;
        pthread_mutex_unlock(&decode_lock);
        return;
    }


    //6.遍历avFormatContext 因为avFormatContext 中有流数据相关信息 此处我们要找的是音频流
    // nb_streams: Number of elements in avFormatContext.streams
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        LOG_D("iterate stream");
        if (avFormatContext->streams[i] != NULL) {
            if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                //将读取的到音频相关数据都存放到AudioController中
                if (audioController == NULL) {
                    audioController = new AudioController(javaBridge, playStatus,
                                                          avFormatContext->streams[i]->codecpar->sample_rate);
                    audioController->streamIndex = i;
                    audioController->avCodecParameters = avFormatContext->streams[i]->codecpar;
                    audioController->timeBase = avFormatContext->streams[i]->time_base;

                    //4.15 获取总时长  转换为秒
                    audioController->duration = avFormatContext->duration / AV_TIME_BASE;
                    duration = audioController->duration;


                }
            }
        } else {
            LOG_D("stream is null");
        }

    }
    //7.找到编码器
    AVCodec *avCodec = avcodec_find_decoder(audioController->avCodecParameters->codec_id);

    if (!avCodec) {
        LOG_E("error to find codec");
        if (javaBridge != NULL) {
            javaBridge->onCallErrMessage(TASK_THREAD, 1003, "error to find codec");
        }
        isDecodeTerminate = true;
        pthread_mutex_unlock(&decode_lock);
        return;
    }
    LOG_D("prepare Task join");

    //8.分配解码器上下文
    audioController->avCodecContext = avcodec_alloc_context3(avCodec);
    if (!audioController->avCodecContext) {
        javaBridge->onCallErrMessage(TASK_THREAD, 1008, "can not alloc codeccontext");
        isDecodeTerminate = true;
        pthread_mutex_unlock(&decode_lock);
        return;
    }


    //9.将解码器参数写入到解码器上下文中
    if (avcodec_parameters_to_context(audioController->avCodecContext,
                                      audioController->avCodecParameters) < 0) {
        LOG_E("error to fill avcodec param to avcodec context");
        if (javaBridge != NULL) {
            javaBridge->onCallErrMessage(TASK_THREAD, 1004,
                                         "error to fill avcodec param to avcodec context");
        }
        isDecodeTerminate = true;
        pthread_mutex_unlock(&decode_lock);
        return;
    }


    //10.用avCodec 实例化AVCodeContext
    if (avcodec_open2(audioController->avCodecContext, avCodec, 0) != 0) {
        LOG_E("error open audio stream");
        if (javaBridge != NULL) {
            javaBridge->onCallErrMessage(TASK_THREAD, 1005,
                                         "error to init avCodecContext by avCodec");
        }
        isDecodeTerminate = true;
        pthread_mutex_unlock(&decode_lock);
        return;
    }
    if (javaBridge != NULL) {
        if (playStatus != NULL && !playStatus->exit) { ;
            javaBridge->onCallPrepared(TASK_THREAD);
        } else {
            isDecodeTerminate = true;
        }

    }
    pthread_mutex_unlock(&decode_lock);

}

void FFmpegController::startPlay() {
    if (audioController == NULL) {
        return;
    }
    LOG_E("start to play form native")
    //TODO 执行播放操作，此时可能没有数据，但是不妨碍。只要将数据解析到缓存中，audioController可以从队列中取数据，
    audioController->playMusic();

    while (playStatus != NULL && !playStatus->exit) {

        if (playStatus->seekByUser) {
            continue;
        }

        //TODO seek 保证队列中有值
        if (audioController->bufferQueue->getQueueSize() > 40) {
            continue;
        }

        AVPacket *avPacket = av_packet_alloc();
        //TODO 解码过程就是从AvFrameContext中获取被压缩的数据包AVPacket，因此可以把这些数据包存入到队列中，完成边解码边播放的功能

        pthread_mutex_lock(&decode_lock);
        int ret = av_read_frame(avFormatContext, avPacket);
        pthread_mutex_unlock(&decode_lock);
        if (ret == 0) {

            //注意AVFormatContext中包含了多个流信息，每个流信息在AVMediaType中去匹配，比如音频AUDIO 视频VIDEO ，所以在这里比对角标是很有必要的。
            //Question：如何知道av_read_frame(avFormatContext,avPacket)读取的packet是属于哪个流的数据包呢，
            //Answer: 我们通过在AVFormatContext,遍历所有的流信息，然后根据AVMediaType匹配是否为对应的流，然后记下各自流的角标，因此在从AVFormatContext读取的时候就可以对应的上了。
            if (avPacket->stream_index == audioController->streamIndex) {

                audioController->bufferQueue->putPacketToQueue(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }


        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            while (playStatus != NULL && !playStatus->exit) {
                if (audioController->bufferQueue->getQueueSize() > 0) {
                    continue;
                } else {
                    playStatus->exit = true;
                    break;
                }
            }
            //TODO 退出循环
            break;
        }


    }
    if (javaBridge != NULL) {
        javaBridge->onCallComplete(TASK_THREAD, true);
    }
    isDecodeTerminate = true;
}

void FFmpegController::pausePlay() {
    if (audioController != NULL) {
        audioController->pause();
    }

}

void FFmpegController::resumePlay() {

    if (audioController != NULL) {
        audioController->resume();
    }
}

void FFmpegController::release() {
    LOG_E("begin free the ffmpeg");
    if (playStatus != NULL && playStatus->exit) {
        return;
    }
    playStatus->exit = true;
    int sleepCount = 0;

    pthread_mutex_unlock(&decode_lock);

    while (!isDecodeTerminate) {

        if (sleepCount > 1000) {
            isDecodeTerminate = true;

        }
        sleepCount++;
        av_usleep(1000 * 10);//10毫秒

    }

    if (audioController != NULL) {
        LOG_E("free audioController");
        audioController->release();
        delete (audioController);
        audioController = NULL;
    }

    if (avFormatContext != NULL) {
        LOG_E("free avFormatContext");
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = NULL;
    }
    if (javaBridge != NULL) {
        LOG_E("free javaBridge");
        javaBridge = NULL;
    }
    if (playStatus != NULL) {
        delete (playStatus);
        playStatus = NULL;
    }

    pthread_mutex_unlock(&decode_lock);

}

void FFmpegController::seek(uint64_t second) {
    if (duration <= 0) {
        return;
    }
    //valid seek and continue the resample not  effect the process
    if (second > 0 && second <= duration) {
        if (audioController != NULL) {
            //flag start
            playStatus->seekByUser = true;

            //clear the buffer
            audioController->bufferQueue->clearQueue();
            //reset time
            audioController->clock = 0;
            audioController->last_time = 0;
            //lock
            pthread_mutex_lock(&seek_lock);
            //seek the file
            int64_t realtime = second * AV_TIME_BASE;
            int seek_result = avformat_seek_file(avFormatContext, -1,
                                                 INT64_MIN, realtime, INT64_MAX, 0);
            LOG_E("seek_result %d：", seek_result);
            if (seek_result < 0) {
                audioController->javaBridge->onCallErrMessage(TASK_THREAD, 1006,
                                                              "error to seek the file");
            }
            //unlock
            pthread_mutex_unlock(&seek_lock);

            //flag end
            playStatus->seekByUser = false;
        }

    }

}
