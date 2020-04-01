#include <jni.h>
#include <string>

#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"


#include "android/log.h"

#include "RecordBuffer.h"

#define TAG "HASH"
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);

//录制大小设置位4096

#define RECORDER_FRAME_SIZE (2048)
static unsigned recorderSize = RECORDER_FRAME_SIZE * 2;


// 引擎接口
static SLObjectItf engineObj = NULL;

//引擎对象
static SLEngineItf engineEngine = NULL;



//需要录音 所以需要录音对象和接口

//录音接口
SLObjectItf recorderObj = NULL;

//录音对象
SLRecordItf recordItf = NULL;


//缓冲队列对象
SLAndroidSimpleBufferQueueItf androidSimpleBufferQueueItf = NULL;

//PCM 数据读取队列
RecordBuffer *recordBuffer = NULL;

//PCM文件
FILE *pcmFile;


bool isFinishRecord = false;

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
Java_com_android_media_opensladuio_MainActivity_n_1record(JNIEnv *env, jobject instance,
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




    SLDataLocator_AndroidSimpleBufferQueue locator_androidSimpleBufferQueue ={
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2
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
Java_com_android_media_opensladuio_MainActivity_n_1stop(JNIEnv *env, jobject instance) {
    if (recordItf != NULL) {
        LOGE("停止录音了....")
        isFinishRecord = true;

    }

}