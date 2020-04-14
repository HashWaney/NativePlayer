/**
 * Created by Hash on 2020-04-13.
 */


#include "FFmpegController.h"


void *prepareCallBack(void *data) {
    FFmpegController *controller = (FFmpegController *) data;
    controller->prepareTask();
    pthread_exit(&controller->prepareThread);


}


FFmpegController::FFmpegController(JavaBridge *javaBridge) {
    this->javaBridge = javaBridge;
    playStatus = new PlayStatus();


}

FFmpegController::~FFmpegController() {
    delete playStatus;
}


void FFmpegController::prepare(const char *url) {

    //TODO 在C++里面开启一个子线程读取url信息

    pthread_create(&prepareThread, NULL, prepareCallBack, this);


}

//2.准备工作（解码操作）
void FFmpegController::prepareTask() {

    //1.注册所有的编解码器 在libavformat/avformat.h
    av_register_all();

    //2.网络初始化
    avformat_network_init();

    //3.分配avFormatContext 该上下文包含了流媒体的信息：比如是音频流 编解码 视频流
    avFormatContext = avformat_alloc_context();
    //4.打开url地址 读取url信息 &avFormatContext 这个是引用  也就是指向指针的指针
    if (avformat_open_input(&avFormatContext, url, NULL, NULL) != 0) {
        LOG_E("can not open error url %s", url);
        return;
    }

    //5.判断读取的媒体信息是否包含流信息
    if (avformat_find_stream_info(avFormatContext, NULL)) {
        LOG_E("can not find streams from %s", url);
        return;
    }

    //6.遍历avFormatContext 因为avFormatContext 中有流数据相关信息 此处我们要找的是音频流
    // nb_streams: Number of elements in avFormatContext.streams
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            //将读取的到音频相关数据都存放到AudioController中
            if (audioController == NULL) {
                audioController = new AudioController(
                        avFormatContext->streams[i]->codecpar->sample_rate);
                audioController->streamIndex = i;
                audioController->avCodecParameters = avFormatContext->streams[i]->codecpar;
                audioController->timeBase = avFormatContext->streams[i]->time_base;

            }
        }
    }
    //7.找到编码器
    AVCodec *avCodec = avcodec_find_decoder(audioController->avCodecParameters->codec_id);

    if (!avCodec) {
        LOG_E("error to find codec");
        return;;
    }

    //8.分配解码器上下文
    audioController->avCodecContext = avcodec_alloc_context3(avCodec);
    if (!audioController->avCodecContext) {
        LOG_E("error to allocate avCodecContext");
        return;
    }

    //9.将解码器参数写入到解码器上下文中
    if (avcodec_parameters_to_context(audioController->avCodecContext,
                                      audioController->avCodecParameters) < 0) {
        LOG_E("error to fill avcodec param to avcodec context");
        return;
    }


    //10.用avCodec 实例化AVCodeContext
    if (avcodec_open2(audioController->avCodecContext, avCodec, NULL) != 0) {
        LOG_E("error open audio stream");
        return;
    }

}

void FFmpegController::startPlay() {
    if (audioController == NULL) {
        LOG_E("audioController is null");
        return;
    }
    //TODO 执行播放操作，此时可能没有数据，但是不妨碍。只要将数据解析到缓存中，audioController可以从队列中取数据，
    audioController->playMusic();

    while (playStatus != NULL && !playStatus->exit) {
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(avFormatContext, avPacket)) {

        }


    }


}
