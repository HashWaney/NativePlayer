//
// Created by 王庆 on 2020-03-30.
//

#include "SimpleAvCodec.h"


SimpleAvCodec::SimpleAvCodec(const char *_url, CallJavaBridge *callJavaBridge) {
    this->_url = _url;
    this->callJavaBridge = callJavaBridge;

}

SimpleAvCodec::~SimpleAvCodec() {

}

pthread_t preparedThread;


void *preparedCallback(void *data) {
    LOGD("callback enter ")

    SimpleAvCodec *avCodec = (SimpleAvCodec *) data;


    avCodec->decodePrepared();


    pthread_exit(&preparedThread);
}


void SimpleAvCodec::prepared() {
    LOGD("prepared fun star")


    pthread_create(&preparedThread, NULL, preparedCallback, this);

    LOGD("prepared fun end")

}

void SimpleAvCodec::decodePrepared() {

    LOGD("decoder Prepared init ")
    //1.初始化组件（复用器 解复用器，解码器）
    av_register_all();
    //2.网络初始化
    avformat_network_init();


    avCodecContext = avformat_alloc_context();
    //3。打开一个网络音频流
    if (avformat_open_input(&avCodecContext, _url, NULL, NULL) != 0) {
        LOGD("can not open the resource :%s", _url);
        return;
    }

    //4。获取网络音频流信息
    if (avformat_find_stream_info(avCodecContext, NULL) < 0) {
        LOGD("can not get stream info")
        return;
    }

    //5。遍历网络音频流信息    Number of elements in AVFormatContext.streams.

    for (int i = 0; i < avCodecContext->nb_streams; ++i) {
        //该判断就是在找到网络音频流的类型是AUDIO类型
        if (avCodecContext->streams[i] != NULL) {
            if (avCodecContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                avCodecParameters = avCodecContext->streams[i]->codecpar;
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
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);

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
    // TODO
    while (1) {
        //死循环，分配内存
        AVPacket *avPacket = av_packet_alloc();

        if (av_read_frame(avCodecContext, avPacket) == 0) { //读取成功
            if (avPacket->stream_index == streamIndex) {
                count++;
                LOGD("解码第 %d 帧, 角标为 %d", count, streamIndex);
            }

            //释放内存 如果此处释放内存，会造成内存泄漏。
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;


        } else {
            //失败释放内存
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            LOGD("读取失败跳出循环")
            break;

        }

    }


}
