//
// Created by 王庆 on 2020-03-30.
//

#include "SimpleAvCodec.h"


pthread_t preparedThread;
SwrContext *swrContext = NULL;


FILE *pcmFile = fopen("/sdcard/music11.m4u", "w");

SimpleAvCodec::SimpleAvCodec(AVPlayStatus *avPlayStatus, const char *_url,
                             CallJavaBridge *callJavaBridge) {
    this->_url = _url;
    this->callJavaBridge = callJavaBridge;
    this->avPlayStatus = avPlayStatus;
    avPacketQueue = new AVPacketQueue(avPlayStatus);

}

SimpleAvCodec::~SimpleAvCodec() {

}


void *preparedCallback(void *data) {
    LOGD("callback enter ")

    SimpleAvCodec *avCodec = (SimpleAvCodec *) data;


    avCodec->preparedDecode();


    pthread_exit(&preparedThread);
}


void SimpleAvCodec::prepared() {
    LOGD("prepared fun star")


    pthread_create(&preparedThread, NULL, preparedCallback, this);

    LOGD("prepared fun end")

}

void SimpleAvCodec::preparedDecode() {
    if (avPlayStatus->exit) {
        LOGD("当前状态是退出状态 无需进行解码准备阶段。");
        return;
    }


    LOGD("decoder Prepared init ")
    //1.初始化组件（复用器 解复用器，解码器）
    av_register_all();
    //2.网络初始化
    avformat_network_init();


    avFormatContext = avformat_alloc_context();
    int err_code;
    char buf[1024];
    //3。打开一个网络音频流
    if ((err_code = avformat_open_input(&avFormatContext, _url, NULL, NULL)) != 0) {
        av_strerror(err_code, buf, 1024);
        LOGD("can not open the resource :%s ,%d(%s)", _url, err_code, buf);

        return;
    }

    //4。获取网络音频流信息
    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGD("can not get stream info")
        return;
    }

    //5。遍历网络音频流信息    Number of elements in AVFormatContext.streams.

    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        //该判断就是在找到网络音频流的类型是AUDIO类型
        if (avFormatContext->streams[i] != NULL) {
            if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                avCodecParameters = avFormatContext->streams[i]->codecpar;
                sample_rate = avFormatContext->streams[i]->codecpar->sample_rate;
                out_buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);
                //角标记录是为了在进行解码的时候，从AVPacket包中匹配到type类型等于AUDIO音频流，解码也是为了通过读取AVPacket中的音频
                streamIndex = i;
                LOGD("类型为AUDIO的 角标为 %d:  当前数据帧的采样率：%d ", streamIndex, sample_rate);

            }
        }

    }

    AVCodec *avCodec = avcodec_find_decoder(avCodecParameters->codec_id);

    if (!avCodec) {
        LOGD("can not find AVCodec")
        return;
    }

    // 分配AVCodecContext 内存空间
    avCodecContext = avcodec_alloc_context3(avCodec);

    //将AVCodecParameters的数据填充到AVCodecContext 中
    if (avcodec_parameters_to_context(avCodecContext, avCodecParameters) < 0) {
        LOGD("can not fill AvCodecContext form AvCodecParameters")
        return;
    }


    //6。打开一个解码器
    if (avcodec_open2(avCodecContext, avCodec, NULL) != 0) {
        LOGD("can not open the codec")
        return;
    };

    //该处是一个子线程。
    callJavaBridge->callPrepared(CHILD_THREAD);


}


/**
 * 进行解码操作
 */
void SimpleAvCodec::startDecode() {

    // TODO  此处需要进行队列缓存操作，现在采取的方式是先解码AVPacket数据包，
    //  比较耗时，因此将解码的数据放在队列中，然后后续播放操作直接从队列中读取
    playAudio();
    while (avPlayStatus != NULL && !avPlayStatus->exit) {
        //死循环，分配内存
        AVPacket *avPacket = av_packet_alloc();

        if (av_read_frame(avFormatContext, avPacket) == 0) { //读取成功
            if (avPacket->stream_index == streamIndex) {

                //TODO  模拟入队操作
                avPacketQueue->putAvPacket(avPacket);
            } else {
                //释放内存 如果此处释放内存，会造成内存泄漏。
                releaseResource();
            }


        } else {
            //失败释放内存
            releaseResource();
            //如果当前未退出，继续循环
            while (avPlayStatus != NULL && !avPlayStatus->exit) {
                if (avPacketQueue->getAVPacketQueueSize() > 0) {
                    continue;
                } else {
                    LOGD("读取失败跳出循环");
                    avPlayStatus->exit = true;
                    break;
                }
            }
        }

    }
    //模拟出队列操作
    while (avPacketQueue->getAVPacketQueueSize() > 0) {
        AVPacket *packet = av_packet_alloc();
        avPacketQueue->getAvPacket(packet);
        //TODO 释放分配的内存
        releaseResource();
    }
    LOGD("解码完成了");


}


//重采样以及播放音频 回调函数
void *decodePlay(void *data) {
    SimpleAvCodec *avCodec = (SimpleAvCodec *) data;
    avCodec->initOpenSlEs();
    pthread_exit(&avCodec->play_thread);
}


//重采样 ---> 采样对象是  <-----解码出来的音频帧（AVframe）<---队列中的AVPacket<--通过解码器解码得到的数据。
/**
 * @return 获取每一次重采样的数据。
 */
int SimpleAvCodec::resample() {


    while (avPlayStatus != NULL && !avPlayStatus->exit) {
        //TODO 当前队列中是否有数据
//        if (avPacketQueue->getAVPacketQueueSize() == 0) { //队列中没有数据，加载中
//            if (!callJavaBridge->isLoading) {
//                callJavaBridge->isLoading = true;
//                callJavaBridge->loadStatus(true, CHILD_THREAD);
//            }
//
//        } else {
//            if (callJavaBridge->isLoading) {
//                callJavaBridge->isLoading = false;
//                callJavaBridge->loadStatus(false, CHILD_THREAD);
//            }
//            continue;
//        }
        if (avPacketQueue->getAVPacketQueueSize() == 0) {
            LOGD("当前队列中数据为0 ");
            isLoading = true;
            if (callJavaBridge != NULL) {
                callJavaBridge->callLoadStatus(isLoading, CHILD_THREAD);
            }


        } else {
            LOGD("数据不为0")
            isLoading = false;
            if (callJavaBridge != NULL) {
                callJavaBridge->callLoadStatus(isLoading, CHILD_THREAD);
            }

        }

        avPacket = av_packet_alloc();
        if (avPacketQueue->getAvPacket(avPacket) != 0) { //从队列中获取失败
            //释放当前分配的Avpacket
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            //继续下一次操作取操作
            continue;

        }
        /**
         * 1.将队列中读取的avpacket 存入到AvCodecContext
         */
        mallocResult = avcodec_send_packet(avCodecContext, avPacket);
        if (mallocResult != 0) //分配失败
        {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        /**
         * 2.将AvCodecContext中的读取到的avpacket存储到AvFrame中 得到解码之后的音频帧
         */
        avFrame = av_frame_alloc();
        mallocResult = avcodec_receive_frame(avCodecContext, avFrame);
        if (mallocResult == 0) { //分配成功
            //如果音频帧中声道个数配置了，但是声道布局没有配置，则需要给声道布局一个默认的配置
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {

                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
                LOGD("channel_layout 布局为%lld", avFrame->channel_layout);

            } else if (avFrame->channels == 0 &&
                       avFrame->channel_layout > 0) { //如果声道布局配置了，但是声道个数没有配置，给声道数一个默认的配置
                //Return the number of channels in the channel layout.
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
                LOGD("channels in the channel_layout %d", avFrame->channels);
            }



            ////////////////////////////    上述步骤都是为了得到AVPacket中的AVFrame 然后对AVFrame进行重采样。 ////////////////////////////
            /// ////////////////////////////     ////////////////////////////

            /**
             * *swr_alloc_set_opts(
             *                        struct SwrContext *s,
                                      int64_t out_ch_layout,  //输出声道布局
                                      enum AVSampleFormat out_sample_fmt, //输出采样位宽 16个字节
                                      int out_sample_rate, //输出采样率
                                      int64_t  in_ch_layout, //输入声道布局
                                      enum AVSampleFormat  in_sample_fmt,//输入采样位宽
                                      int  in_sample_rate, //输入采样率
                                      int log_offset,
                                      void *log_ctx)

                       输入的声道布局比如是单声道--->   输出的声道布局可以设置为立体声输出
                       输入的采样位宽比如说8个字节-->   输出的采样位宽就可以设置为16个字节
                       输入的采样率比如是44100Hz -->   输出的采样率也可以设置为44100Hz

                       采样率： 每秒对音频数据采样的个数
                       采样位数：存储采样数据的位数
                       输出声道：单声道。立体声
             */
            swrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO, //声道布局（立体声）AV_CH_FRONT_LEFT|AV_CH_FRONT_RIGHT 左前/右前
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL,
                    NULL
            );

            if (!swrContext || swr_init(swrContext) < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                //TODO 此处不要将avFrame置为空以及释放，会导致在取采样率的时候avFrame 为空指针异常。
                //经过测试avFrame=NULL注释掉，不会导致程序崩溃，我猜想应该是avFrame分配的内存被释放了，但是引用还保留，所以
                av_frame_free(&avFrame);
                av_free(avFrame);
//                avFrame = NULL;
                swr_free(&swrContext);
                continue;

            }

            /////////////////////////采样转换函数，将原始的压缩数据进行重采样操作////////////////////////////////////////

            int sample_rate = swr_convert(swrContext,
                                          &out_buffer, //输出缓冲区 （数据涞源为AVFrame的数据，也就是说该缓冲区会存储数据）
                                          avFrame->nb_samples,
                                          (const uint8_t **) (avFrame->data),
                                          avFrame->nb_samples
            );
            //输出声道个数
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            int sample_format = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            data_size = sample_format * out_channels * sample_rate;
//            LOGD("size of %d -> out_channel %d -> sample_format %d-> all byte %d", sample_rate,
//                 out_channels, sample_format, data_size);
            fwrite(out_buffer, 1, data_size, pcmFile);
            releaseResource();

            //TODO break 跳出循环，不然死循环。
            break;
        } else { //分配失败
            releaseResource();
            continue;
        }


    }
    fclose(pcmFile);
    return data_size;


}


void SimpleAvCodec::playAudio() {
    LOGD("进入重采样了");
    pthread_create(&play_thread, NULL, decodePlay, this);


}

void SimpleAvCodec::releaseResource() {

    if (avPacket != NULL) {
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }

    if (avFrame != NULL) {
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
    }
    if (swrContext != NULL) {
        swr_free(&swrContext);
    }
}


//缓冲队列回调
void pcmPlayCallback(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context) {
    SimpleAvCodec *simpleAvCodec = (SimpleAvCodec *) context;

    int bufferSize = simpleAvCodec->resample();
    if (bufferSize > 0) {
        SLresult sLresult = (*simpleAvCodec->androidSimpleBufferQueueItf)->Enqueue(
                simpleAvCodec->androidSimpleBufferQueueItf, simpleAvCodec->out_buffer, bufferSize);
        if (SL_RESULT_SUCCESS != sLresult) {
            LOGD("播放完成，跳出循环")

        }
    }


}


//初始化OpenSl 环境
void SimpleAvCodec::initOpenSlEs() {

    SLresult result;
    //1. 创建引擎对象
    slCreateEngine(&engineObj, 0, 0, 0, 0, 0);
    (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineEngine);


    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean require[1]{SL_BOOLEAN_FALSE};

    //2.创建混音器
    result = (*engineEngine)->CreateOutputMix(engineEngine,
                                              &outputMixObj,
                                              1,
                                              ids,
                                              require);


    //实例化混音器接口
    result = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGD("混音器实例化失败");
        return;
    }


    //创建混音器对象
    result = (*outputMixObj)->GetInterface(outputMixObj, SL_IID_ENVIRONMENTALREVERB, &outputMixItf);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("混音器接口获取失败");
        return;
    }
    //设置混音器环境变量
    (*outputMixItf)->SetEnvironmentalReverbProperties(outputMixItf, &environmentalReverbSettings);


    SLDataLocator_AndroidSimpleBufferQueue android_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2
    };
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, //指定播放的是Pcm数据
            2,//声道数
            (SLuint32) (getSampleRateOfPerFrame(sample_rate)), //动态分配采样率 从每一帧数据中获取真实采样率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位宽2个字节
            SL_PCMSAMPLEFORMAT_FIXED_16,//位宽2个字节
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声 （前左，前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };


    //配置Audio数据源 就是缓冲队列中的存储的pcm数据，以及pcm数据指定的格式，声道，采样率，位宽等信息
//    SLDataSource audioSrc ={ 缓冲队列, 格式};
    SLDataSource audioSrc = {
            &android_queue,
            &format_pcm
    };



    //输出：通过混音器进行pcm音频数据的输出

    SLDataLocator_OutputMix outputMix = {
            SL_DATALOCATOR_OUTPUTMIX,
            outputMixObj
    };
    SLDataSink dataSink = {
            &outputMix, NULL

    };


    const SLInterfaceID interfaceID[3] = {
            SL_IID_BUFFERQUEUE,
            SL_IID_VOLUME,
            SL_IID_EFFECTSEND

    };
    const SLboolean req[3] = {
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE
    };



    //3.创建播放器
    result = (*engineEngine)->CreateAudioPlayer(
            engineEngine,
            &pcmPlayerObj,
            &audioSrc,
            &dataSink,
            3,
            interfaceID,
            req
    );



    //3.1 实例化播放器接口
    (*pcmPlayerObj)->Realize(pcmPlayerObj, SL_BOOLEAN_FALSE);

    //3.2 获取播放器对象
    (*pcmPlayerObj)->GetInterface(pcmPlayerObj, SL_IID_PLAY, &playItf);

    //4.创建缓冲区
    (*pcmPlayerObj)->GetInterface(pcmPlayerObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                  &androidSimpleBufferQueueItf);

    //4.1 缓冲区接口回调

    /**
     * RegisterCallback(SLAndroidSimpleBufferQueueItf self,slAndroidSimpleBufferQueueCallback callback,void* pContext);
     *
     * slAndroidSimpleBufferQueueCallback --- > 是一个回调函数
     * 定义为 返回值是void 参数为SLAndroidSimpleBufferQueueItf 和 一个void*
     * typedef void (SLAPIENTRY *slAndroidSimpleBufferQueueCallback)(
            SLAndroidSimpleBufferQueueItf caller,
            void *pContext);
     */


    (*androidSimpleBufferQueueItf)->RegisterCallback(androidSimpleBufferQueueItf, pcmPlayCallback,
                                                     this);

    //5.获取音量接口
    (*pcmPlayerObj)->GetInterface(pcmPlayerObj, SL_IID_VOLUME, &volumeItf);
    //TODO volumeItf 可以设置音量大小


    //6.设置播放状态
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);


    //7.主动调用一次回调函数开启pcm数据播放
    pcmPlayCallback(androidSimpleBufferQueueItf, this);


}


int SimpleAvCodec::getSampleRateOfPerFrame(int sample_rate) {
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

//
void SimpleAvCodec::pause() {
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PAUSED);


    }

}

void SimpleAvCodec::replay() {
    if (playItf != NULL) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    }


}




