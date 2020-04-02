//
// Created by 王庆 on 2020-03-30.
//

#include "SimpleAvCodec.h"


pthread_t preparedThread;
SwrContext *swrContext = NULL;


FILE *pcmFile = fopen("/sdcard/music.pcm", "w");

SimpleAvCodec::SimpleAvCodec(AVPlayStatus *avPlayStatus, const char *_url,
                             CallJavaBridge *callJavaBridge) {
    this->_url = _url;
    this->callJavaBridge = callJavaBridge;
    this->avPlayStatus = avPlayStatus;
    avPacketQueue = new AVPacketQueue(avPlayStatus);
    buffer = (uint8_t *)av_malloc(44100*2*2);
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
    //3。打开一个网络音频流
    if (avformat_open_input(&avFormatContext, _url, NULL, NULL) != 0) {
        LOGD("can not open the resource :%s", _url);
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
                //角标记录是为了在进行解码的时候，从AVPacket包中匹配到type类型等于AUDIO音频流，解码也是为了通过读取AVPacket中的音频
                streamIndex = i;
                LOGD("类型为AUDIO的 角标为 %d:", streamIndex);

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
    int count = 0;
    // TODO  此处需要进行队列缓存操作，现在采取的方式是先解码AVPacket数据包，
    //  比较耗时，因此将解码的数据放在队列中，然后后续播放操作直接从队列中读取
    playAudio();
    while (avPlayStatus != NULL && !avPlayStatus->exit) {
        //死循环，分配内存
        AVPacket *avPacket = av_packet_alloc();

        if (av_read_frame(avFormatContext, avPacket) == 0) { //读取成功
            if (avPacket->stream_index == streamIndex) {
                count++;
                LOGD("解码第 %d 帧, 角标为 %d", count, streamIndex);
                //TODO  模拟入队操作
                avPacketQueue->putAvPacket(avPacket);
            } else {
                //释放内存 如果此处释放内存，会造成内存泄漏。
//            该Packet有引用计数（packet->buf不为空）
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
    avCodec->resample();
    pthread_exit(&avCodec->play_thread);
}


//重采样 ---> 采样对象是  <-----解码出来的音频帧（AVframe）<---队列中的AVPacket<--通过解码器解码得到的数据。
int SimpleAvCodec::resample() {

    //@TEST 将采样的读取的缓存数据写入到文件中。


    while (avPlayStatus != NULL && !avPlayStatus->exit) {
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
//                av_frame_free(&avFrame);
//                av_free(avFrame);
//                avFrame = NULL;
                swr_free(&swrContext);
                continue;

            }

            /////////////////////////采样转换函数，将原始的压缩数据进行重采样操作////////////////////////////////////////

            int sample_rate = swr_convert(swrContext,
                                          &buffer,
                                          avFrame->nb_samples,
                                          (const uint8_t **) (avFrame->data),
                                          avFrame->nb_samples
            );
            //输出声道个数
            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            int sample_format = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            data_size = sample_format * out_channels * sample_rate;
            LOGD("size of %d -> out_channel %d -> sample_format %d-> all byte %d", sample_rate,
                 out_channels, sample_format, data_size);
            fwrite(buffer, 1, data_size, pcmFile);

            releaseResource();


        } else { //分配失败
            releaseResource();
            continue;
        }


    }
    fclose(pcmFile);
    return data_size;


}


void SimpleAvCodec::playAudio() {
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




