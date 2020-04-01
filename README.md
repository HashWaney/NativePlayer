## OpenSL ES 播放音频文件（mp3 ，pcm， url)

  环境配置：
  1。创建引擎
  2。创建混音器
  3。配置PCM格式信息
  4。初始化播放器
  5。


## 异常 排查


     `
        AudioSnk: data locator type 0x800007be not allowed

     `
     该处设置为SLDataLocator_AndroidBufferQueue 报错 应该设置为SLDataLocator_AndroidSimpleBufferQueue

     bug 分析： data locator 就应该仔细检查SLDataLocator 是否配置错误。



    `
        pAudioSnk: endianness=0  字节序 = 0  应该是没有配置SL_BYTEORDER_LITTLEENDIAN
    `


        SLDataFormat_PCM format_pcm = {
                    SL_DATAFORMAT_PCM, //pcm 格式
                    2,  //2声道
                    SL_SAMPLINGRATE_44_1, // 采样率位44100hz
                    SL_PCMSAMPLEFORMAT_FIXED_16, //位宽16个字节
                    SL_PCMSAMPLEFORMAT_FIXED_16, //2声道 设置两次
                    SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, //左前声道，右前声道
                    SL_BYTEORDER_LITTLEENDIAN //
                参考java.nio.ByteOrder---> 属性： LITTLE_ENDIAN  // 按照此顺序，多字节值的字节顺序是从最低有效位到最高有效位的
            };



### 流程分析：详细分析 查看 native-lib.cpp 文件

  -  该处只做简要重要流程分析

    // 引擎接口
    static SLObjectItf engineObj = NULL;

    //引擎对象
    static SLEngineItf engineEngine = NULL;


    //录音器接口
    static SLObjectItf recorderObj=NULL;

    //录音器对象
    static SLRecordItf recorderItf=NULL;

    //缓冲区接口
    static SLObjectItf androidSimpleQueueObj=NULL;


    //缓冲区对象
    static  SLAndroidSimpleBufferQueueItf androidSimpleBufferQueueItf=NULL;





    .../ 引擎相关的创建步骤


    1。创建引擎接口 slCreateEngine(&engineObj, 0, NULL, 0, NULL, NULL);

    2.实例化引擎接口 (*engineObj)->Realize(engineObj,SL_BOOLEAN_FALSE);

    3.创建引擎对象  (*engineObj)->GetInterface(engineObj,SD_IID_ENGINE,&engineEngine);


    .../录音器相关配置 包括麦克风IO设备 格式（pcm 采样率，声道，位宽，字节排序） 以及缓冲区创建

    1.创建录音器(切记： 是通过上述的引擎对象来创建录音接口）
            （*engineEngine)->CreateAudioRecorder(engineEngine,
                        &recorderObj,
                        麦克风设备数据,
                        输出数据(缓冲队列,pcm 格式),
                        1,
                        指定的SL_IID_ANDROIDSIMPLEBUFFERQUEUE，
                        SL_BOOLEAN_TRUE


                );

    2.获取录音器对象
            (*enginEngine)->GetInterface(recorderObj,SL_IID_RECORD,&recorderItf);



    3.获取缓冲区对象

            (*enginEngine)->GetInterface(androidSimpleQueueObj,SL_IID_ANDROIDSIMPLEBUFFERQUEUE,&androidSimpleBufferQueueItf);



    .../缓冲区将读取的数据加入到队列中,注册回调，在回调中进行连续录制音频

           androidSimpleBufferQueue->Enqueu();


           androidSimpleBufferQueue->RegisterCallback();

    ..../录音器需要根据录制情况设置录音状态

            state =SL_RECORDSTATE_STOPPED;
                   SL_RECORDSTATE_PAUSED
                   SL_RECORDSTATE_RECORDING
         (*recorderItf)->SetRecordState(recorderItd,state);




