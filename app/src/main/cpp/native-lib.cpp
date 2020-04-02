#include <jni.h>
#include <string>

#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include "PlayCallback.h"


#include "android/log.h"

#include "RecordBuffer.h"

#define TAG "HASH"
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);

//录制大小设置位4096

#define RECORDER_FRAME_SIZE (2048)
static unsigned recorderSize = RECORDER_FRAME_SIZE * 2;

#define Sample (44100)
static unsigned readBufferSize = Sample * 2 * 2;

// 引擎接口
static SLObjectItf engineObj = NULL;

//引擎对象
static SLEngineItf engineEngine = NULL;


PlayCallback *playCallback = NULL;

////////////////////////////////////////////录音功能///////////////////////////////////////////////////////////

//需要录音 所以需要录音对象和接口

//录音接口
SLObjectItf recorderObj = NULL;

//录音对象
SLRecordItf recordItf = NULL;


///////////////////////////////////////////////////////////////////////////////////////////////////////

//缓冲队列对象
SLAndroidSimpleBufferQueueItf androidSimpleBufferQueueItf = NULL;

//PCM 数据读取队列
RecordBuffer *recordBuffer = NULL;

//PCM文件
static FILE *pcmFile;


bool isFinishRecord = false;


///////////////////////////////////////////播放功能/////////////////////////////////////////////////////////////
//2。混音器对象
SLObjectItf outputMixObj = NULL;

SLEnvironmentalReverbItf outputMixItf = NULL;

SLEnvironmentalReverbSettings environmentalReverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

//3.pcm播放器接口
SLObjectItf pcmPlayerObj = NULL;
SLPlayItf playItf = NULL;


//5.音量接口
SLVolumeItf volumeItf = NULL;

uint8_t *out_buff;

void *buffer;
//////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * 缓冲区的回调
 * @param caller
 * @param pContext
 */
void bufferQueueCallBack(SLAndroidSimpleBufferQueueItf caller,
                         void *pContext) {
    // 获取缓冲区的数据，以每次写入一个
    /**
     *（1）buffer:是一个指针,对fwrite来说,是要获取数据的地址;
     * (2)size:要写入内容的单字节数;
     * (3)count:要进行写入size字节的数据项的个数;
     * (4)stream:目标文件指针;
     * (5)返回实际写入的数据项个数count。
     */
    fwrite(recordBuffer->getCurrentRecordBuffer(), 1, recorderSize, pcmFile);

    if (!isFinishRecord) {
        (*androidSimpleBufferQueueItf)->Enqueue(androidSimpleBufferQueueItf,
                                                recordBuffer->getNewRecordBuffer(), recorderSize);

    } else {
        LOGE("停止录音了.. 外部用户杀掉进程了")
        //设置当前录制状态为停止
        (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_PAUSED);
        //关闭文件
        fclose(pcmFile);

    }

}


extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_opensladuio_PlayStatus_n_1record(JNIEnv *env, jobject instance,
                                                        jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    // 打开文件
    pcmFile = fopen(path, "w");

    //1.1创建引擎对象
    SLresult result;
    //创建引擎接口
    result = slCreateEngine(&engineObj, 0, NULL, 0, NULL, NULL);
    //1.2实例化engine接口
    result = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    //1.3拿到engine对象 才能真正的去调用
    result = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineEngine);


    //datasource 数据涞源 IO （麦克风）
    SLDataLocator_IODevice locator_ioDevice = {
            SL_DATALOCATOR_IODEVICE,
            SL_IODEVICE_AUDIOINPUT,
            SL_DEFAULTDEVICEID_AUDIOINPUT,
            NULL
    };
    // 设置IO设备（麦克风）

    SLDataSource audioSrc = {&locator_ioDevice, NULL};

    //设置缓冲队列


    //TODO 该处设置为SLDataLocator_AndroidBufferQueue 报错 应该设置为SLDataLocator_AndroidSimpleBufferQueue




    SLDataLocator_AndroidSimpleBufferQueue locator_androidSimpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2
    };


    //设置录制规格： PCM ，2声道， 采样率44100，位宽16bit
    /**
     * SL_DATAFORMAT_PCM,
     * 2,
     * SL_SAMPLINGRATE_44_1,
       SL_PCMSAMPLEFORMAT_FIXED_16,
       SL_PCMSAMPLEFORMAT_FIXED_16,
       SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
       SL_BYTEORDER_LITTLEENDIAN
     */
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16, //2声道 设置两次
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, //左前声道，右前声道
            SL_BYTEORDER_LITTLEENDIAN
    };


    //输出 数据 到一个pcm文件中。
    SLDataSink audioSink = {&locator_androidSimpleBufferQueue, &format_pcm};

    //
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};

    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    //2.1 创建录音器
    result = (*engineEngine)->CreateAudioRecorder(engineEngine,
                                                  &recorderObj,
                                                  &audioSrc,
                                                  &audioSink,
                                                  1,
                                                  id,
                                                  req);

    if (SL_RESULT_SUCCESS != result) {
        LOGE("录音器创建失败")
        return;
    }

    //2.2 实例化对象
    result = (*engineObj)->Realize(recorderObj, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("录音器对象实例化不成功");
        return;
    }
    //2.3 获取录音器接口
    result = (*engineObj)->GetInterface(recorderObj, SL_IID_RECORD, &recordItf);

    result = (*engineObj)->GetInterface(recorderObj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                        &androidSimpleBufferQueueItf);


    //PCM buffer 队列
    recordBuffer = new RecordBuffer(RECORDER_FRAME_SIZE * 2);



    //3.加入队列
    result = (*androidSimpleBufferQueueItf)->Enqueue(androidSimpleBufferQueueItf,
                                                     recordBuffer->getNewRecordBuffer(),
                                                     recorderSize);

    result = (*androidSimpleBufferQueueItf)->RegisterCallback(
            androidSimpleBufferQueueItf, bufferQueueCallBack, NULL);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("队列创建失败");
        return;
    }
    LOGE("开始进行录音了....");


    /**
     *
         Recording states
        #define SL_RECORDSTATE_STOPPED 	((SLuint32) 0x00000001)
        #define SL_RECORDSTATE_PAUSED	((SLuint32) 0x00000002)
        #define SL_RECORDSTATE_RECORDING	((SLuint32) 0x00000003)
     */

    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);

    env->ReleaseStringUTFChars(path_, path);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_opensladuio_PlayStatus_n_1stop(JNIEnv *env, jobject instance) {
    if (recordItf != NULL) {
        LOGE("停止录音了....")
        isFinishRecord = true;

    }


}


int getPcmData(void **buffer) {
    int size = 0;
    while (!feof(pcmFile)) {
        size = fread(out_buff, 1, readBufferSize, pcmFile);
//        if (out_buff == NULL) {
//            LOGE("说明读取的内容已经读完了");
//        } else {
//            LOGE("正在读取中");
//        }
        // out_buffer 写入的数据赋值给buffer指向的内存地址 指针
        *buffer = out_buff;
        //每调用一次 读取一次
        break;

    }
    return size;
}


void pcmPlayCallback(SLAndroidSimpleBufferQueueItf slAndroidSimpleBufferQueueItf, void *context) {

    //通过将out_buffer分配的读取空间 读取的数据存储道buffer中，数据相当于是存储在buffer中
    int size = getPcmData(&buffer);
    if (buffer != NULL) {
        SLresult result = (*androidSimpleBufferQueueItf)->Enqueue(androidSimpleBufferQueueItf,
                                                                  buffer, size);
        if (SL_RESULT_SUCCESS != result) {
            LOGE("回调给Java开始");
            if (playCallback != NULL) {
                LOGE("调用Java方法")
                playCallback->callMethod();

            }
            LOGE("回调给Java结束");

        }

    }

}

//全局变量
JavaVM *jvm;


JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {

    // TODO 通过测试JNIEnv的环境是否OK
    jvm = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }


    return JNI_VERSION_1_6;


}


extern "C"
JNIEXPORT void JNICALL
Java_com_android_media_opensladuio_PlayStatus_n_1play(JNIEnv *env, jobject instance,
                                                      jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    if (playCallback == NULL) {
        LOGE("实例化playCallback");
        playCallback = new PlayCallback(jvm, env->NewGlobalRef(instance));

    }

    ///////////////////////////////读取文件////////////////////////////////////////////////////

    pcmFile = fopen(path, "r");

    if (pcmFile == NULL) {
        LOGE("%s", "file open error");
        return;
    }

    //设置每一秒钟读取的缓冲区大小 44100Hz 2 声道 位宽位16bit -2字节
    out_buff = (uint8_t *) malloc(readBufferSize);



    ////////////////////////////////播放器搭建/////////////////////////////////////////////////

    SLresult result;
    //1. 创建引擎对象
    slCreateEngine(&engineObj, 0, 0, 0, 0, 0);
    (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineEngine);

    //SL_IID_ENVIRONMENTALREVERB
//    const SLInterfaceID ids[1] = {SL_IID_ENGINECAPABILITIES};
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
        LOGE("混音器实例化失败");
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
            SL_SAMPLINGRATE_44_1,//采样率
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
            engineEngine, &pcmPlayerObj, &audioSrc,
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
                                                     NULL);

    //5.获取音量接口
    (*pcmPlayerObj)->GetInterface(pcmPlayerObj, SL_IID_VOLUME, &volumeItf);
    //TODO volumeItf 可以设置音量大小


    //6.设置播放状态
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);


    //7.主动调用一次回调函数开启pcm数据播放
    pcmPlayCallback(androidSimpleBufferQueueItf, NULL);


    env->ReleaseStringUTFChars(path_, path);
}