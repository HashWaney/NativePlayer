# NativePlayer
A native player  for audio  play


## 空指针异常 记录：

   `Fatal signal 11 (SIGSEGV), code 1, fault addr 0x0 in tid 31286 `

    ```

        pthread_t* thread;

        pthread_create(thread,NULL,callback,this);

    ```

    上诉代码直接崩溃,原因如下：
        指针引用不能访问地址；
        pthread_t 是线程的标识符

        pthread_create(thread,....)
        thread 被定义一个指针，不能直接访问地址，
        而pthread_create(....)在创建指针的过程中，第一个参数是线程标志的地址，也就是需要通过&thread的方式来访问thread的地址，
        但是thread被定义为指针，指针是不能直接访问地址的，因此触发了崩溃。


    正确做法：
        声明一个pthread_t thread;
        因此我们只需要声明一个线程标识符，thread, 通过&thread获取thread的地址，然后通过pthread_create()方法来创建线程。
        pthread_create(&thread,....);


## AVPacket中的内存管理


    AVPacket实际上可用看作一个容器，它本身并不包含压缩的媒体数据，而是通过data指针引用数据的缓存空间。

    当从一个Packet去创建另一个Packet的时候，有两种情况：
    -   两个Packet的data引用的是同一数据缓存空间，这时候要注意数据缓存空间的释放问题。
    -   两个Packet的data引用不同的数据缓存空间，每个Packet都有数据缓存空间的copy。

    对于多个Packet共享同一个缓存空间，FFmpeg使用的引用计数的机制（reference-count）。
    当有新的Packet引用共享的缓存空间时，就将引用计数+1；当释放了引用共享空间的Packet，就将引用计数-1；引用计数为0时，就释放掉引用的缓存空间。


   `av_packet_ref(AVPacket* dst,AVPacket* src)`

    创建一个src->data的新的引用计数。如果src已经设置了引用计数发（src->buffer不为空），则直接将其引用计数+1；
    如果src没有设置引用计数（src->buffer为空），则为dst创建一个新的引用计数buf，并复制src->data到buf->buffer中。
    最后，复制src的其他字段到dst中。

    `av_packet_unref(AVPacket* avpacket)`


## Native 调试


    /Users/wangqing/Library/Android/ndk/android-ndk-r17c/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-addr2line

    通过NDK包下自带工具arm-linux-androideabi-addr2line 定位C++代码有问题的行数

    arm-linux-androideabi-addr2line -C -f -e /.../lib-xxx.so 00009631

    上述/..../..lib-xx.so 是生成的so的路径，00009631 是出错的内存地址


    `
         Build fingerprint: 'honor/FRD-AL00/HWFRD:8.0.0/HUAWEIFRD-AL00/540(C00):user/release-keys'
        2020-03-31 13:46:47.481 24010-24010/? A/DEBUG: Revision: '0'
        2020-03-31 13:46:47.481 24010-24010/? A/DEBUG: ABI: 'arm'
        2020-03-31 13:46:47.481 24010-24010/? A/DEBUG: pid: 23617, tid: 24005, name: Thread-10  >>> com.android.media.player <<<
        2020-03-31 13:46:47.481 24010-24010/? A/DEBUG: signal 6 (SIGABRT), code -6 (SI_TKILL), fault addr --------
        2020-03-31 13:46:47.485 24010-24010/? A/DEBUG: Abort message: 'Invalid address 0xe3a2c580 passed to free: value not allocated'
        2020-03-31 13:46:47.485 24010-24010/? A/DEBUG:     r0 00000000  r1 00005dc5  r2 00000006  r3 00000008
        2020-03-31 13:46:47.485 24010-24010/? A/DEBUG:     r4 00005c41  r5 00005dc5  r6 ca206ec0  r7 0000010c
        2020-03-31 13:46:47.485 24010-24010/? A/DEBUG:     r8 7ffff000  r9 ed9fe388  sl ca207020  fp cd3d6200
        2020-03-31 13:46:47.485 24010-24010/? A/DEBUG:     ip 00000000  sp ca206eb0  lr f0e794e7  pc f0eaa8fc  cpsr 20000010
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG: backtrace:
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #00 pc 0004b8fc  /system/lib/libc.so (tgkill+12)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #01 pc 0001a4e3  /system/lib/libc.so (abort+54)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #02 pc 0001eab9  /system/lib/libc.so (__libc_fatal+24)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #03 pc 0006c739  /system/lib/libc.so (ifree+608)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #04 pc 0006c951  /system/lib/libc.so (je_free+72)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #05 pc 00009631  /data/app/com.android.media.player-XKEGvgmeognlISL0ZRobAw==/lib/arm/libnative-lib.so (_ZN13AVPacketQueue11getAvPacketEP8AVPacket+192)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #06 pc 0000902b  /data/app/com.android.media.player-XKEGvgmeognlISL0ZRobAw==/lib/arm/libnative-lib.so (_ZN13SimpleAvCodec11startDecodeEv+198)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #07 pc 000088d9  /data/app/com.android.media.player-XKEGvgmeognlISL0ZRobAw==/lib/arm/libnative-lib.so (Java_com_android_media_nativeplayerlib_AudioPlayer_startDecode+40)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #08 pc 003e4679  /system/lib/libart.so (art_quick_generic_jni_trampoline+40)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #09 pc 003e0931  /system/lib/libart.so (art_quick_invoke_stub_internal+64)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #10 pc 003e4ea3  /system/lib/libart.so (art_quick_invoke_stub+226)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #11 pc 000ac2d9  /system/lib/libart.so (_ZN3art9ArtMethod6InvokeEPNS_6ThreadEPjjPNS_6JValueEPKc+140)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #12 pc 001f27fb  /system/lib/libart.so (_ZN3art11interpreter34ArtInterpreterToCompiledCodeBridgeEPNS_6ThreadEPNS_9ArtMethodEPKNS_7DexFile8CodeItemEPNS_11ShadowFrameEPNS_6JValueE+238)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #13 pc 001edd71  /system/lib/libart.so (_ZN3art11interpreter6DoCallILb0ELb0EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE+576)
        2020-03-31 13:46:47.497 24010-24010/? A/DEBUG:     #14 pc 003ca871  /system/lib/libart.so (MterpInvokeVirtual+440)
        2020-03-31 13:46:47.498 24010-24010/? A/DEBUG:     #15 pc 003d2c14  /system/lib/libart.so (ExecuteMterpImpl+14228)
        2020-03-31 13:46:47.498 24010-24010/? A/DEBUG:     #16 pc 001d5351  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadEPKNS_7DexFile8CodeItemERNS_11ShadowFrameENS_6JValueEb+340)
        2020-03-31 13:46:47.498 24010-24010/? A/DEBUG:     #17 pc 001da6a3  /system/lib/libart.so (_ZN3art11interpreter33ArtInterpreterToInterpreterBridgeEPNS_6ThreadEPKNS_7DexFile8CodeItemEPNS_11ShadowFrameEPNS_6JValueE+142)
        2020-03-31 13:46:47.498 24010-24010/? A/DEBUG:     #18 pc 001edd5b  /system/lib/libart.so (_ZN3art11interpreter6DoCallILb0ELb0EEEbPNS_9ArtMethodEPNS_6ThreadERNS_11ShadowFrameEPKNS_11InstructionEtPNS_6JValueE+554)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #19 pc 003cb4a3  /system/lib/libart.so (MterpInvokeInterface+1210)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #20 pc 003d2e14  /system/lib/libart.so (ExecuteMterpImpl+14740)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #21 pc 001d5351  /system/lib/libart.so (_ZN3art11interpreterL7ExecuteEPNS_6ThreadEPKNS_7DexFile8CodeItemERNS_11ShadowFrameENS_6JValueEb+340)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #22 pc 001da5f1  /system/lib/libart.so (_ZN3art11interpreter30EnterInterpreterFromEntryPointEPNS_6ThreadEPKNS_7DexFile8CodeItemEPNS_11ShadowFrameE+92)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #23 pc 003c0fbd  /system/lib/libart.so (artQuickToInterpreterBridge+944)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #24 pc 003e46f1  /system/lib/libart.so (art_quick_to_interpreter_bridge+32)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #25 pc 003e0931  /system/lib/libart.so (art_quick_invoke_stub_internal+64)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #26 pc 003e4ea3  /system/lib/libart.so (art_quick_invoke_stub+226)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #27 pc 000ac2d9  /system/lib/libart.so (_ZN3art9ArtMethod6InvokeEPNS_6ThreadEPjjPNS_6JValueEPKc+140)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #28 pc 00334bb1  /system/lib/libart.so (_ZN3artL18InvokeWithArgArrayERKNS_33ScopedObjectAccessAlreadyRunnableEPNS_9ArtMethodEPNS_8ArgArrayEPNS_6JValueEPKc+52)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #29 pc 00335a2d  /system/lib/libart.so (_ZN3art35InvokeVirtualOrInterfaceWithJValuesERKNS_33ScopedObjectAccessAlreadyRunnableEP8_jobjectP10_jmethodIDP6jvalue+320)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #30 pc 00353ae9  /system/lib/libart.so (_ZN3art6Thread14CreateCallbackEPv+892)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #31 pc 00048c1f  /system/lib/libc.so (_ZL15__pthread_startPv+22)
        2020-03-31 13:46:47.503 24010-24010/? A/DEBUG:     #32 pc 0001b09f  /system/lib/libc.so (__start_thread+32)
        2020-03-31 13:46:47.541 749-749/? E/wificond: Failed to get NL80211_STA_INFO_CNAHLOAD
        2020-03-31 13:46:47.541 1136-1306/? E/WificondControl: Noise: -88, Snr: 0, Chload: -1



    `


    由上述崩溃日志可知，地址为  00009631  0000902b 000088d9 三处地址定位出问题的C++代码 出问题的so库就是libnative-lib.so


        #05 pc 00009631  /data/app/com.android.media.player-XKEGvgmeognlISL0ZRobAw==/lib/arm/libnative-lib.so (_ZN13AVPacketQueue11getAvPacketEP8AVPacket+192)
        #06 pc 0000902b  /data/app/com.android.media.player-XKEGvgmeognlISL0ZRobAw==/lib/arm/libnative-lib.so (_ZN13SimpleAvCodec11startDecodeEv+198)
        #07 pc 000088d9  /data/app/com.android.media.player-XKEGvgmeognlISL0ZRobAw==/lib/arm/libnative-lib.so (Java_com_android_media_nativeplayerlib_AudioPlayer_startDecode+40)








## 关于对象释放以及对象内存空间释放问题

     一般原则不要处处都进行对象的释放，可参考SimpleAcCodec.cpp 文件中，

        ```
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


        ```
        ```
            //TODO avFrame 在上述代码中进行释放操作，那么在使用avFrame对象就会空指针异常。切记，不要处处都进行指针内存释放，可以在功能完成之后，进行统一的释放操作。
            int sample_rate = swr_convert(swrContext,
                                          &buffer,
                                          avFrame->nb_samples,
                                          (const uint8_t **) (avFrame->data),
                                          avFrame->nb_samples

         ```











