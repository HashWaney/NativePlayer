package cn.hash.mm.nativelib;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import cn.hash.mm.nativelib.bean.AudioInfoBean;
import cn.hash.mm.nativelib.bean.MuteType;
import cn.hash.mm.nativelib.listener.OnCurrentAudioDbListener;
import cn.hash.mm.nativelib.listener.OnPauseResumeListener;
import cn.hash.mm.nativelib.listener.OnPlayCompleteListener;
import cn.hash.mm.nativelib.listener.OnPlayErrorListener;
import cn.hash.mm.nativelib.listener.OnPlayLoadListener;
import cn.hash.mm.nativelib.listener.OnPrepareListener;
import cn.hash.mm.nativelib.listener.OnTimeInfoListener;
import cn.hash.mm.nativelib.util.NativeLibLogUtil;

/**
 * Created by Hash on 2020-04-13.
 */


public class PlayController {

    private static final String TAG = PlayController.class.toString();

    private OnPrepareListener onPrepareListener;

    private OnPlayLoadListener onPlayLoadListener;

    private OnPauseResumeListener onPauseResumeListener;

    private OnTimeInfoListener timeInfoListener;

    private OnPlayErrorListener onPlayErrorListener;

    private OnPlayCompleteListener onPlayCompleteListener;

    private OnCurrentAudioDbListener dbListener;

    private static boolean playNext = false;

    private static int duration = -1;

    private static int current_volume = 40;

    private static float pitch = 1.0f;
    private static float speed = 1.0f;

    private static boolean isStartRecord = false;

    private static boolean initMediaCodec = false;

    private static int audioSampleRate = 0;

    private static int db = 0;
    private static MuteType muteType = MuteType.MUTE_TYPE_CENTER;


    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec");
        System.loadLibrary("avdevice");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
    }


    //播放源
    private static String resource;

    private static AudioInfoBean audioInfoBean = null;



    public void setResource(String resource) {
        this.resource = resource;
    }

    public void setOnPrepareListener(OnPrepareListener prepareListener) {
        this.onPrepareListener = prepareListener;
    }

    public void setOnPauseResumeListener(OnPauseResumeListener onPauseResumeListener) {
        this.onPauseResumeListener = onPauseResumeListener;
    }


    public void setTimeInfoListener(OnTimeInfoListener timeInfoListener) {
        this.timeInfoListener = timeInfoListener;
    }

    public void setOnPlayErrorListener(OnPlayErrorListener errorListener) {
        this.onPlayErrorListener = errorListener;
    }

    public void setOnPlayLoadListener(OnPlayLoadListener onPlayLoadListener) {
        this.onPlayLoadListener = onPlayLoadListener;
    }

    public void setOnPlayCompleteListener(OnPlayCompleteListener onPlayCompleteListener) {
        this.onPlayCompleteListener = onPlayCompleteListener;
    }


    public void setOnCurrentAudioDbListener(OnCurrentAudioDbListener dbListener) {
        this.dbListener = dbListener;
    }

    public PlayController() {

    }


    //1.准备阶段

    public void prepare() {
        if (TextUtils.isEmpty(resource)) {
            Log.d(TAG, String.format("url %d is null", resource));
            return;
        }


        //开启一个子线程
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepare(resource);
            }
        }).start();


    }

    //2.开始播放
    public void startPlay() {
        if (TextUtils.isEmpty(resource)) {
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                setVolume(current_volume);
                setMuteType(muteType);
                setSpeed(speed);
                setPitch(pitch);
                n_startPlay();
            }
        }).start();

    }


    //3.暂停
    public void pausePlay() {
        n_pause();
        if (onPauseResumeListener != null) {
            onPauseResumeListener.onPauseOrResume(true);
        }

    }

    //4.恢复
    public void resumePlay() {
        n_resume();
        if (onPauseResumeListener != null) {
            onPauseResumeListener.onPauseOrResume(false);
        }

    }


    //5.停止
    public void stopAndRelease(final int nextPage) {
        //playNext 重置bean
        audioInfoBean = null;
        duration = -1;
        db = 0;
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop(nextPage);
            }
        }).start();


    }

    //6.进度
    public void seek(int second) {
        n_seek(second);
    }


    //7.下一首
    public void playNext(String url) {
        resource = url;
        playNext = true;
        //释放当前资源
        stopAndRelease(1);

    }

    //8.获取播放时长
    public int getDuration() {
        if (duration < 0) {
            duration = n_duration();
        }
        return duration;
    }

    //9.设置音量
    public void setVolume(int volume) {
        if (volume >= 0 && volume <= 100) {
            current_volume = volume;
            n_setvolume(volume);
        }
    }

    //10.获取音量

    public int getVolume() {

        return current_volume;
    }

    //11.设置声道
    public void setMuteType(MuteType m) {
        muteType = m;
        n_muteType(m.getValue());


    }

    //12.设置变速
    public void setSpeed(float s) {
        speed = s;
        n_setSpeed(speed);

    }

    //13.设置变调
    public void setPitch(float p) {
        pitch = p;
        n_setPitch(pitch);
    }

    //14. 边播放边录制
    public void startRecord(boolean start, File recordFile) {
        if (!initMediaCodec) {
            initMediaCodec = true;
            isStartRecord = start;
            //如果采样率大于0 说明这个时候有音频数据，认定为有效录制
            audioSampleRate = n_getSampleRate();
            if (audioSampleRate > 0) {
                Log.d(TAG, "sample is valid" + audioSampleRate);
                initMediaCodec(audioSampleRate, recordFile);
            }
            n_startRecord(isStartRecord);
        }

    }


    public void pauseRecord(boolean record) {
        isStartRecord = record;
        n_pauseRecord(isStartRecord);
        Log.d(TAG, "暂停录制:"+isStartRecord);


    }

    public void resumeRecord(boolean record) {
        isStartRecord = record;
        n_resumeRecord(isStartRecord);
        Log.d(TAG, "继续录制");
    }

    public void stopRecord(boolean record) {
        if (initMediaCodec) {
            isStartRecord = record;
            n_stopRecord(isStartRecord);
            releaseMediaCodec();
            Log.d(TAG, "完成录制");
        }


    }

    private void releaseMediaCodec() {
        if (encoder == null) {
            return;
        }

        try {
            outputStream.close();
            outputStream = null;

            encoder.stop();
            encoder.release();
            encoder = null;
            bufferInfo = null;
            encodeFormat = null;
            initMediaCodec = false;
            Log.d(TAG, "释放资源");

        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (outputStream != null) {
                try {
                    outputStream.close();
                } catch (Exception e) {
                    e.printStackTrace();

                }
                outputStream = null;
            }

        }


    }

    ////////////////////////mediaCodec aac ///////////////////////////////////
    /**
     * 背景知识，acc 需要添加头部信息，在编码aac 裸流数据的时候，如果没有为aac增加头信息是不能正常播放的，
     * 头信息一般是7个字节，头信息一般会包含采样率，声道，数据帧的长度等信息，这样编码器才能解析
     */
    private int aacSampleRateIndex = 4; // 默认是 44.1Khz对应的采样率下标
    private MediaFormat encodeFormat = null; //媒体格式
    private FileOutputStream outputStream = null; //文件输出流，将编码之后的数据写入到文件中
    private MediaCodec.BufferInfo bufferInfo = null;//用于更新缓冲区元数据信息
    private int perPcmSize = 0; //每一帧pcm的大小
    private byte[] outByteBuffer = null;//输出的aac的缓冲区

    private MediaCodec encoder = null;//编码器用于将pcm数据编码为aac


    private void initMediaCodec(int sampleRate, File recordFile) {

        try {
            //1.根据采样率换算成aac头信息 采样率下标sampling_frequency_index
            aacSampleRateIndex = getADTSSampleRate(sampleRate);
            //2。创建Audio格式
            encodeFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate,
                    2);
            //设置传输比特率
            encodeFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            //设置AAC profile
            encodeFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            //设置最大的输入
            encodeFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);

            //初始化缓冲区信息
            bufferInfo = new MediaCodec.BufferInfo();
            //创建编码器
            encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            if (encoder == null) {
                NativeLibLogUtil.logD("encoder is null");
                return;
            }
            //component as an encoder. MediaCodec.CONFIGURE_FLAG_ENCODER 将mediaCodec 视为一个编码器
            encoder.configure(encodeFormat, null, null,
                    MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(recordFile);
            //开启编码器
            encoder.start();
        } catch (IOException e) {
            Log.d(TAG, "create media Codec:" + e.getLocalizedMessage());
            e.printStackTrace();
        }


    }

    /**
     * 根据每一帧返回的采样率 获取到对应的ADTS中对应的采样率角标
     *
     * @param sampleRate
     * @return
     */
    private int getADTSSampleRate(int sampleRate) {
        int rate = 4;
        switch (sampleRate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }


    ///////////////////native callback////////////////////////////////////
    public void prepareCallBackFormNative() {
        if (onPrepareListener != null) {
            onPrepareListener.prepare();
        }

    }


    public void callLoadFromNative(boolean isLoad) {
        if (onPlayLoadListener != null) {
            onPlayLoadListener.onLoad(isLoad);

        }
    }

    public void callCompleteFromNative(boolean isComplete) {
        // TODO: 2020-04-16 播放完成 回收资源
        if (onPlayCompleteListener != null) {
            onPlayCompleteListener.complete(isComplete);
        }
    }


    public void callNextAfterInvokeN_Stop() {
        if (playNext) {
            playNext = false;
            prepare();
        }
    }

    //获取当前播放时长和总时长
    public void callTimeInfoFromNative(int currentTime, int totalTime) {
        if (timeInfoListener != null) {
            if (audioInfoBean == null) {
                audioInfoBean = new AudioInfoBean();
            }
            audioInfoBean.setCurrentTime(currentTime);
            audioInfoBean.setTotalTime(totalTime);
            timeInfoListener.getCurrentTimeInfo(audioInfoBean);
        }
    }

    //获取当前db值
    public void callDbFromNative(int audioDb) {
        db = audioDb;
        if (dbListener != null) {
            dbListener.setCurrentDb(db);
        }
    }

    //错误回调
    public void errMessageFromNative(int errorCode, String errorMessage) {
        // TODO: 2020-04-16  播放出错 回收资源
        stopAndRelease(-1);

        if (onPlayErrorListener != null) {
            onPlayErrorListener.onError(errorCode, errorMessage);
        }
    }


    /**
     * 获取pcm数据进行编码成aac
     *
     * @param size        音频帧的大小
     * @param inputBuffer 音频帧的输入字节数据
     */
    public void encodePcmToAAC(int size, byte[] inputBuffer) {
        if (inputBuffer != null && encoder != null) {
            int inputBufferIndex = encoder.dequeueInputBuffer(-1);
            if (inputBufferIndex >= 0) {
                // TODO: 2020-04-21  从encoder取出的ByteBuffer的缓冲区最大设置为了4096
                //  由MediaFormat.KEY_INPUT_MAX_SIZE指定 如果输入的buffer大小超过了这个缓冲区大小，则会抛出BufferOverflowException
                ByteBuffer byteBuffer = encoder.getInputBuffers()[inputBufferIndex];
                // TODO: 2020-04-21 暂时不删除以下代码 用于分析 buffer的容量与设置的format的关系
                int capacity = byteBuffer.capacity();
                int inputMaxSize = encodeFormat.getInteger(MediaFormat.KEY_MAX_INPUT_SIZE);

                byteBuffer.clear();
                byteBuffer.put(inputBuffer);
                encoder.queueInputBuffer(inputBufferIndex, 0, size, 0, 0);
            }

            int index = encoder.dequeueOutputBuffer(bufferInfo, 0);
            while (index >= 0) {
                try {
                    perPcmSize = bufferInfo.size + 7;
                    outByteBuffer = new byte[perPcmSize];

                    ByteBuffer byteBuffer = encoder.getOutputBuffers()[index];
                    byteBuffer.position(bufferInfo.offset);
                    byteBuffer.limit(bufferInfo.offset + bufferInfo.size);

                    addADTSHeader(outByteBuffer, perPcmSize, aacSampleRateIndex);

                    byteBuffer.get(outByteBuffer, 7, bufferInfo.size);
                    byteBuffer.position(bufferInfo.offset);
                    outputStream.write(outByteBuffer, 0, perPcmSize);

                    encoder.releaseOutputBuffer(index, false);
                    index = encoder.dequeueOutputBuffer(bufferInfo, 10000);
                    outByteBuffer = null;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * 添加adts头部信息
     *
     * @param outByteBuffer      头信息的byte容器
     * @param perPcmSize         数据帧的大小
     * @param aacSampleRateIndex 采样率角标
     */
    private void addADTSHeader(byte[] outByteBuffer, int perPcmSize, int aacSampleRateIndex) {

        int profile = 2;//AAC LC
        int freIndex = aacSampleRateIndex;//采样率下标
        int channelConfig = 2;//CPE

        outByteBuffer[0] = (byte) 0xFF;
        outByteBuffer[1] = (byte) 0xF9;
        outByteBuffer[2] = (byte) (((profile - 1) << 6) + (freIndex << 2) + (channelConfig >> 2));
        outByteBuffer[3] = (byte) (((channelConfig & 3) << 6) + (perPcmSize >> 11));
        outByteBuffer[4] = (byte) ((perPcmSize & 0x7FF) >> 3);
        outByteBuffer[5] = (byte) (((perPcmSize & 7) << 5) + 0x1F);
        outByteBuffer[6] = (byte) 0xFC;


    }


    /////////////////////////native///////////////////////////////////
    public native void n_prepare(String resource);

    public native void n_startPlay();

    public native void n_pause();

    public native void n_resume();

    public native void n_stop(int nextPage);

    public native void n_seek(int seconds);

    public native int n_duration();

    public native void n_setvolume(int volume);

    public native void n_muteType(int muteType);

    public native void n_setSpeed(float speed);

    public native void n_setPitch(float pitch);

    public native int n_getSampleRate();

    public native void n_startRecord(boolean record);

    public native void n_stopRecord(boolean record);


    public native void n_pauseRecord(boolean record);


    public native void n_resumeRecord(boolean record);


}
