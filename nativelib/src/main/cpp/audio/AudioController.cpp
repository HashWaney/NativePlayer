/**
 * Created by Hash on 2020-04-13.
 */


#include "AudioController.h"

AudioController::AudioController(JavaBridge *javaBridge, PlayStatus *playStatus, int sample_rate) {
    this->playStatus = playStatus;
    this->javaBridge = javaBridge;
    this->sample_rate = sample_rate;
    bufferQueue = new BufferQueue(playStatus);
    //TODO 为buffer 分配内存空间，按照采样率*声道数*位宽 的形式
    receiveDataFromFrameBuffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);


}

AudioController::~AudioController() {
    delete bufferQueue;
}


void *play_musicCallback(void *data) {
    AudioController *audioController = (AudioController *) data;

    //TODO 初始化OpenSL ES 用于播放音频文件
    audioController->initOpenSLES();

    pthread_exit(&audioController->playThread);
}


void pcmPlayBufferQueueCallBack(SLAndroidSimpleBufferQueueItf bf, void *data) {
    AudioController *audioController = (AudioController *) data;
    if (audioController != NULL) {
        int bufferSize = audioController->resampleAudio();
        if (bufferSize > 0) {

            // 计算时间
            audioController->clock +=
                    bufferSize / ((double) (audioController->sample_rate * 2 * 2));

            if (audioController->clock - audioController->last_time >= 0.1) {
                audioController->last_time = audioController->clock;
                //回调给Java层时间
                audioController->javaBridge->onCallTimeInfo(TASK_THREAD, audioController->clock,
                                                            audioController->duration);
            }


            (*audioController->androidSimpleBufferQueueItf)->Enqueue(
                    audioController->androidSimpleBufferQueueItf,
                    (char *) audioController->receiveDataFromFrameBuffer, bufferSize);
        }

    }

}

void AudioController::playMusic() {
    pthread_create(&playThread, NULL, play_musicCallback, this);

}

void AudioController::initOpenSLES() {
    SLresult result;

    //1. engine create
    result = slCreateEngine(&engineObj, 0, 0, 0, 0, 0);
    //2. realize
    result = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    //3. get interface
    result = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineItf);


    //4.create mix environmental
    const SLInterfaceID interfaceID[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean interfaceRequest[1] = {SL_BOOLEAN_FALSE};

    result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObj, 1, interfaceID,
                                           interfaceRequest);

    (void) result;
    result = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
    result = (*outputMixObj)->GetInterface(outputMixObj, SL_IID_ENVIRONMENTALREVERB,
                                           &outPutMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outPutMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outPutMixEnvironmentalReverb, &reverbSettings
        );
    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
    SLDataSink audioSink = {&outputMix, 0};

    //5. pcm config
    SLDataLocator_AndroidSimpleBufferQueue androidSimpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2};

    SLDataFormat_PCM dataFormat_pcm = {SL_DATAFORMAT_PCM,//pcm格式
                                       2,// 立体声道
                                       getCurrentSampleRate(sample_rate),
                                       SL_PCMSAMPLEFORMAT_FIXED_16,//位数16
                                       SL_PCMSAMPLEFORMAT_FIXED_16,
                                       SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                       SL_BYTEORDER_LITTLEENDIAN//结束标志
    };

    SLDataSource dataSource = {&androidSimpleBufferQueue, &dataFormat_pcm};

    const SLInterfaceID slInterfaceID[2] = {SL_IID_BUFFERQUEUE,SL_IID_VOLUME};
    const SLboolean requestPlayInter[2] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};
    //6. create player TODO 声音接口注册 不然声音控制失效
    (*engineItf)->CreateAudioPlayer(engineItf, &playObj, &dataSource, &audioSink, 2, slInterfaceID,
                                    requestPlayInter);

    (*playObj)->Realize(playObj, SL_BOOLEAN_FALSE);

    (*playObj)->GetInterface(playObj, SL_IID_PLAY, &playItf);

    (*playObj)->GetInterface(playObj, SL_IID_VOLUME, &pcmVolumePlay);

    (*playObj)->GetInterface(playObj, SL_IID_BUFFERQUEUE, &androidSimpleBufferQueueItf);

    //callback
    (*androidSimpleBufferQueueItf)->RegisterCallback(androidSimpleBufferQueueItf,
                                                     pcmPlayBufferQueueCallBack, this);

    setAudioVolume(currentVolume);
    //play status
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);

    pcmPlayBufferQueueCallBack(androidSimpleBufferQueueItf, this);

}

SLuint32 AudioController::getCurrentSampleRate(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }

    return rate;
}

// 重采样数据进行播放
int AudioController::resampleAudio() {

    data_size = 0;

    while (playStatus != NULL && !playStatus->exit) {

        //use the seek flag to declare i am not effect the process
        if (playStatus->seekByUser) {
            continue;
        }
        if (bufferQueue->getQueueSize() == 0) { //loading
            LOG_E("loading....");
            if (!playStatus->isLoad) {
                playStatus->isLoad = true;
                javaBridge->onCallLoad(TASK_THREAD, true);
            }
            continue;
        } else { //playing
            if (playStatus->isLoad) {
                playStatus->isLoad = false;
                javaBridge->onCallLoad(TASK_THREAD, false);
            }

        }

        avPacket = av_packet_alloc();
        if (bufferQueue->getPacketFromQueue(avPacket) != 0) {
            LOG_E("get packet from queue error ,maybe the queue is empty ")
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        //send packet to avCodecContext
        codecOperateFlag = avcodec_send_packet(avCodecContext, avPacket);
        if (codecOperateFlag != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        avFrame = av_frame_alloc();
        //receive frame from avCodeContext
        codecOperateFlag = avcodec_receive_frame(avCodecContext, avFrame);
        //TODO 待验证 是否正确
        if (codecOperateFlag == 0) {
            //to config the channel_layout and channels
            if (avFrame->channel_layout == 0 && avFrame->channels) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swrContext;
            swrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO, //输出声道布局 立体
                    AV_SAMPLE_FMT_S16, //输出格式
                    avFrame->sample_rate, //输出采样率
                    avFrame->channel_layout, //输入声道布局
                    (AVSampleFormat) avFrame->format, //输入格式
                    avFrame->sample_rate,
                    NULL, NULL);

            if (!swrContext || swr_init(swrContext) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swrContext);
                continue;
            }


            //convert frame to the buffer
            //TODO null point 是因为 receiveDataFormatBuffer没有分配内存空间，所以导致直接崩溃了，因此在构造函数里面进行内存的分配
            //如果不分配，其实也就验证了对一个空变量进行取地址操作 是无法取地址的，因此取地址一般是分配了一块内存空间，那么就有指针指向这一块内存区域，
            //所以对于有内存空间的话变量就可以通过&地址符来取出变量的地址，然后进行赋值等系列操作
            int convert_data = swr_convert(
                    swrContext,
                    &receiveDataFromFrameBuffer,
                    avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,
                    avFrame->nb_samples
            );

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            //采样率*声道数*位宽
            data_size = convert_data * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            now_time = avFrame->pts * av_q2d(timeBase);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;
            //this all and free the var
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            break;
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }


    return data_size;
}

void AudioController::pause() {
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PAUSED);
    }

}

void AudioController::resume() {
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    }
}

void AudioController::release() {

    if (bufferQueue != NULL) {
        delete (bufferQueue);
        bufferQueue = NULL;
    }
    //播放器相关
    if (playObj != NULL) {
        LOG_E("free play obj")
        (*playObj)->Destroy(playObj);
        playObj = NULL;
        playItf = NULL;
        pcmVolumePlay = NULL;
        androidSimpleBufferQueueItf = NULL;
    }

    //混音器
    if (outputMixObj != NULL) {
        LOG_E("free output mix obj")
        (*outputMixObj)->Destroy(outputMixObj);
        outputMixObj = NULL;
        outPutMixEnvironmentalReverb = NULL;
    }
    //引擎相关
    if (engineObj != NULL) {
        LOG_E("free engine obj")
        (*engineObj)->Destroy(engineObj);
        engineObj = NULL;
        engineItf = NULL;
    }
    //存储avframe的buffer
    if (receiveDataFromFrameBuffer != NULL) {
        LOG_E("free buffer");
        free(receiveDataFromFrameBuffer);
        receiveDataFromFrameBuffer = NULL;
    }

    //编码器上下文
    if (avCodecContext != NULL) {
        LOG_E("free avCodecContext")
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    if (playStatus != NULL) {
        LOG_E("free playStatus")
        playStatus = NULL;
    }
    if (javaBridge != NULL) {
        LOG_E("free javaBridge form AudioController");
        javaBridge = NULL;
    }

}

void AudioController::setAudioVolume(int percent) {
    currentVolume = percent;
    if (pcmVolumePlay != NULL) {
        if(percent > 30)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        }
        else if(percent > 25)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        }
        else if(percent > 20)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        }
        else if(percent > 15)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        }
        else if(percent > 10)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        }
        else if(percent > 5)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        }
        else if(percent > 3)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        }
        else if(percent > 0)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        }
        else{
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }

}
