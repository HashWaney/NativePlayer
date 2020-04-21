//
//Created by Hash 2020.4.21
//

#include "AudioController.h"

AudioController::AudioController(JavaBridge *javaBridge, PlayStatus *playStatus, int sample_rate) {
    this->javaBridge = javaBridge;
    this->playStatus = playStatus;
    this->sample_rate = sample_rate;
    bufferQueue = new BufferQueue(playStatus);
    receiveDataFromFrameBuffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);

    sound_touch_out_buffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);

}

AudioController::~AudioController() {

}

void *play_musicCallback(void *data) {
    AudioController *audioController = (AudioController *) data;

    audioController->initOpenSLES();

    pthread_exit(&audioController->playThread);
}


void *pcmToAACBufferCallBack(void *data) {
    AudioController *audioController = (AudioController *) data;

    while (audioController->playStatus && !audioController->playStatus->exit) {

        PcmSplitEntity *pcmSplitEntity = NULL;

        audioController->pcmSplitQueue->getPcmBuffer(&pcmSplitEntity);
        if (pcmSplitEntity == NULL) { continue; }


        if (pcmSplitEntity->bufferSize <= audioController->defaultSize) { //不用分包
            //回调给Java层 将自身数据块大小和缓冲区数据回调出去即可
            if (audioController->record) { //录制标志位
                audioController->javaBridge->onCallPcmToAAC(TASK_THREAD, pcmSplitEntity->bufferSize,
                                                            pcmSplitEntity->buffer);
            }

        } else {
            //进行分包处理
            //多少个包
            int pack_split_num = pcmSplitEntity->bufferSize / audioController->defaultSize;

            //分完包之后剩余的
            int pack_split_left_num = pcmSplitEntity->bufferSize % audioController->defaultSize;

            for (int i = 0; i < pack_split_num; i++) {
                char *buffer = static_cast<char *>(malloc(audioController->defaultSize));
                //拷贝分包数据到buffer中,每次拷贝audioController->defaultSize 个数，位置为p c
                memcpy(buffer, pcmSplitEntity->buffer + i * audioController->defaultSize,
                       audioController->defaultSize);

                //回调buffer给Java
                if (audioController->record) {
                    audioController->javaBridge->onCallPcmToAAC(TASK_THREAD,
                                                                audioController->defaultSize,
                                                                buffer);
                }

                //释放buffer malloc 手动释放内存
                free(buffer);
                buffer = NULL;

            }
            if (pack_split_left_num > 0) {
                char *buffer = static_cast<char *>(malloc(pack_split_left_num));

                //经过了分包之后的位置，应该是当前buffer的位置+分了几个包大小
                //buffer+ n*a 起始点
                //b为余下的数据
                //拷贝到bf中
                memcpy(buffer,
                       pcmSplitEntity->buffer + pack_split_num * audioController->defaultSize,
                       pack_split_left_num);

                //回调给Java 余下的数据块
                if (audioController->record) {
                    audioController->javaBridge->onCallPcmToAAC(TASK_THREAD, pack_split_left_num,
                                                                buffer);
                }

                free(buffer);
                buffer = NULL;
            }
        }

    }


    pthread_exit(&audioController->pcmSplitQueue);
}

void pcmPlayBufferQueueCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    AudioController *audioController = (AudioController *) context;
    if (audioController != NULL) {
        int buffersize = audioController->getSoundTouchData();
        if (buffersize > 0) {
            audioController->clock +=
                    buffersize / ((double) (audioController->sample_rate * 2 * 2));
            if (audioController->clock - audioController->last_time >= 0.1) {
                audioController->last_time = audioController->clock;
                //回调应用层
                audioController->javaBridge->onCallTimeInfo(TASK_THREAD, audioController->clock,
                                                            audioController->duration);
            }
            //回调当前处理的音量的分贝值 buffersize 采样个数 2位宽 2 声道数
            audioController->javaBridge->onCallVolumeDb(TASK_THREAD, audioController->getAudioDb(
                    reinterpret_cast<char *>(audioController->sound_touch_out_buffer),
                    buffersize * 2 * 2));

            //TODO 对此处的数据需要进行分包处理 暂时注释掉，
//            if (audioController->record) {
//                //回调pcm数据进行aac编码
//                audioController->javaBridge->onCallPcmToAAC(TASK_THREAD, buffersize * 2 * 2,
//                                                            audioController->sound_touch_out_buffer);
//            }

            //putPcmBuffer

            audioController->pcmSplitQueue->putPcmBuffer(audioController->sound_touch_out_buffer,
                                                         buffersize * 2 * 2);


            //将缓冲区的数据加入到播放队列中。
            (*audioController->androidSimpleBufferQueueItf)->Enqueue(
                    audioController->androidSimpleBufferQueueItf,
                    (char *) audioController->sound_touch_out_buffer,
                    buffersize * 2 * 2);
        }
    }

}

void AudioController::playMusic() {
    pcmSplitQueue = new PcmSplitQueue(playStatus);
    pthread_create(&playThread, NULL, play_musicCallback, this);
    pthread_create(&pcmBufferThread, NULL, pcmToAACBufferCallBack, this);

}


void AudioController::initOpenSLES() {

    SLresult result;
    result = slCreateEngine(&engineObj, 0, 0, 0, 0, 0);
    result = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    result = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineItf);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineItf)->CreateOutputMix(engineItf, &outputMixObj, 1, mids, mreq);
    (void) result;
    result = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObj)->GetInterface(outputMixObj,
                                           SL_IID_ENVIRONMENTALREVERB,
                                           &outPutMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outPutMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outPutMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            getCurrentSampleRate(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    //SL_IID_PLAYBACKRATE 该属性是自动调整采样率，高采样率 切换到低采样率 会明显的卡顿，opensl es 使用该属性自动转换
    const SLInterfaceID slInterfaceID[4] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE,
                                            SL_IID_MUTESOLO};
    const SLboolean requestPlayInter[4] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
                                           SL_BOOLEAN_TRUE};

    (*engineItf)->CreateAudioPlayer(engineItf, &playObj, &slDataSource, &audioSnk, 3,
                                    slInterfaceID, requestPlayInter);
    //初始化播放器
    (*playObj)->Realize(playObj, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*playObj)->GetInterface(playObj, SL_IID_PLAY, &playItf);
//   获取声音接口
    (*playObj)->GetInterface(playObj, SL_IID_VOLUME, &pcmVolumePlay);
    //获取声道接口
    (*playObj)->GetInterface(playObj, SL_IID_MUTESOLO, &muteSoloItf);

//    注册回调缓冲区 获取缓冲队列接口
    (*playObj)->GetInterface(playObj, SL_IID_BUFFERQUEUE, &androidSimpleBufferQueueItf);
    setAudioVolume(currentVolume);
    setMuteType(currentMuteType);
    //缓冲接口回调
    (*androidSimpleBufferQueueItf)->RegisterCallback(androidSimpleBufferQueueItf,
                                                     pcmPlayBufferQueueCallBack,
                                                     this);
//    获取播放状态接口
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

int AudioController::resampleAudio(void **pcmbuf) {
    data_size = 0;
    while (playStatus != NULL && !playStatus->exit) {

        if (playStatus->seekByUser) {

            //降低CPU 使用率
            av_usleep(1000 * 100);

            continue;
        }
        if (bufferQueue->getQueueSize() == 0)//加载中
        {
            if (!playStatus->isLoad) {
                playStatus->isLoad = true;
                javaBridge->onCallLoad(TASK_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else {
            if (playStatus->isLoad) {
                playStatus->isLoad = false;
                javaBridge->onCallLoad(TASK_THREAD, false);
            }
        }


        if (readFrameFinished) {
            //TODO 设置该变量，就是为了解码一帧avPacket 对应 一帧avFrame。
            avPacket = av_packet_alloc();
            if (bufferQueue->getPacketFromQueue(avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }

            codecOperateFlag = avcodec_send_packet(avCodecContext, avPacket);
            if (codecOperateFlag != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }

        }



        //TODO 将avPacket 转换为 avFrame 才是真正意义上的解码功能。 通过avcodec功能完成解码

        avFrame = av_frame_alloc();
        codecOperateFlag = avcodec_receive_frame(avCodecContext, avFrame);
        if (codecOperateFlag == 0) {
            readFrameFinished = false;
            if (avFrame->channels && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swr_ctx;
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL, NULL
            );
            if (!swr_ctx || swr_init(swr_ctx) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                swr_free(&swr_ctx);
                readFrameFinished = true;
                continue;
            }

            number = swr_convert(
                    swr_ctx,
                    &receiveDataFromFrameBuffer,
                    avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,
                    avFrame->nb_samples
            );

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = number * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            now_time = avFrame->pts * av_q2d(timeBase);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;
            *pcmbuf = receiveDataFromFrameBuffer;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            break;
        } else {
            readFrameFinished = true;
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

    if (pcmSplitQueue != NULL) {
        //把队列中的锁打开，防止阻塞，无法完成释放操作。
        pcmSplitQueue->notifyThread();
        //等待其他线程退出，然后退出pcmBufferThread线程
        pthread_join(pcmBufferThread, NULL);
        pcmSplitQueue->release();
        delete (pcmSplitQueue);
        pcmSplitQueue = NULL;
    }

    if (bufferQueue != NULL) {
        delete (bufferQueue);
        bufferQueue = NULL;
    }

    if (playObj != NULL) {
        (*playObj)->Destroy(playObj);
        playObj = NULL;
        playItf = NULL;
        androidSimpleBufferQueueItf = NULL;
        muteSoloItf = NULL;
        pcmVolumePlay = NULL;
    }

    if (outputMixObj != NULL) {
        (*outputMixObj)->Destroy(outputMixObj);
        outputMixObj = NULL;
        outPutMixEnvironmentalReverb = NULL;
    }

    if (engineObj != NULL) {
        (*engineObj)->Destroy(engineObj);
        engineObj = NULL;
        engineItf = NULL;
    }

    if (receiveDataFromFrameBuffer != NULL) {
        free(receiveDataFromFrameBuffer);
        receiveDataFromFrameBuffer = NULL;
    }

    if (out_buffer != NULL) {
        out_buffer = NULL;
    }

    if (soundTouch != NULL) {
        delete soundTouch;
        soundTouch = NULL;
    }

    if(sound_touch_out_buffer!=NULL)
    {
        free(sound_touch_out_buffer);
        sound_touch_out_buffer=NULL;
    }

    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    if (playStatus != NULL) {
        playStatus = NULL;
    }
    if (javaBridge != NULL) {
        javaBridge = NULL;
    }

}

void AudioController::stop() {
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
    }
}

void AudioController::setAudioVolume(int percent) {
    currentVolume = percent;
    if (pcmVolumePlay != NULL) {
        if (percent > 30) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        } else if (percent > 25) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        } else if (percent > 20) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        } else if (percent > 15) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        } else if (percent > 10) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        } else if (percent > 5) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        } else if (percent > 3) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        } else if (percent > 0) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        } else {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void AudioController::setMuteType(int mute) {
    this->currentMuteType = mute;
    if (muteSoloItf != NULL) {
        if (mute == 0)//right
        {
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 1, false);
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 0, true);
        } else if (mute == 1)//left
        {
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 1, true);
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 0, false);
        } else if (mute == 2)//center
        {
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 1, false);
            (*muteSoloItf)->SetChannelMute(muteSoloItf, 0, false);
        }
    }
}

int AudioController::getSoundTouchData() {
    while (playStatus != NULL && !playStatus->exit) {
        out_buffer = NULL;
        if (finish) {
            finish = false;
            //TODO out_buffer 是用来接收重采样之后的数据的。
            data_size = resampleAudio(reinterpret_cast<void **>(&out_buffer));
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    sound_touch_out_buffer[i] = (out_buffer[i * 2] |
                                                 ((out_buffer[i * 2 + 1]) << 8));
                }
                //将重采样的数据填充到soundtouch 中
                soundTouch->putSamples(sound_touch_out_buffer, number);
                //获取经过soundtouch 进行了变速变调之后的数据
                //拿到重采样的数据 需要对数据进行处理，一般我们只要处理采样个数据就好了，因此不需要位宽（16bit） 声道相关的，data_size/2/2 只处理采样数据
                soundTouchReceiveNum = soundTouch->receiveSamples(sound_touch_out_buffer,
                                                                  data_size / 4);
            } else {
                //将最后一次的数据刷新到
                soundTouch->flush();
            }
        }
        //如果为0 说明从soundTouch 读取的数据为0，继续下一次循环
        if (soundTouchReceiveNum == 0) {

            finish = true;
            continue;
        } else {

            //TODO bug发现 sound_touch_out_buffer ==NULL 判断条件写错了。 这里需要读取重采样的数据帧，如果为空，则需要调用receiveSample方法重新获取。
            // 如果out_buffer里面没有值了，就需要从soundTouch的buffer队列中获取数据，
            if (out_buffer == NULL) {

                //soundTouchReceiveNum 不等于0 说明可以读取到SoundTouch处理的数据。
                soundTouchReceiveNum = soundTouch->receiveSamples(sound_touch_out_buffer,
                                                                  data_size / 4);
                if (soundTouchReceiveNum == 0) {
                    //继续下一次循环，
                    finish = true;
                    continue;
                }
            } else {
                //不为空，暂不做处理。

            }
            //将最终从读取的经过soundTouch处理的数据放入到播放队列中进行播放。
            return soundTouchReceiveNum;
        }
    }
    return 0;
}

void AudioController::setPitch(float pitch) {
    this->pitch = pitch;
    if (soundTouch != NULL) {
        soundTouch->setPitch(pitch);
    }
}

void AudioController::setSpeed(float speed) {
    this->speed = speed;
    if (soundTouch != NULL) {
        soundTouch->setTempo(speed);
    }
}

int AudioController::getAudioDb(char *pcmData, size_t d) {
    int db = 0;
    short int pervalue = 0;
    double sum = 0;
    for (int i = 0; i < d; i += 2) {
        memcpy(&pervalue, pcmData + i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (d / 2);
    if (sum > 0) {
        db = (int) 20.0 * log10(sum);
    }
    return db;
}

void AudioController::startRecord(bool b) {
    this->record = b;


}

void AudioController::pauseRecord(bool record) {
    this->record = record;

}

void AudioController::resumeRecord(bool record) {
    this->record = record;

}

void AudioController::stopRecord(bool record) {
    this->record = record;

}

