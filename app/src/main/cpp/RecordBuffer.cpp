//
// Created by 王庆 on 2020-04-01.
//

#include "RecordBuffer.h"

/**
 * @param bufferSize  设置的缓冲大小
 */
RecordBuffer::RecordBuffer(int bufferSize) {
    // 它是 数组。 【】 大于*
    buffer = new short *[2];
    for (int i = 0; i < 2; i++) {
        buffer[i] = new short[bufferSize];
    }


}

RecordBuffer::~RecordBuffer() {

}

/**
 *
 * @return 当前录制好的PCM数据的buffer，可以写入文件
 */
short *RecordBuffer::getCurrentRecordBuffer() {
    return buffer[index];
}

/**
 * 为什么要index++ : getCurrentRecordBuffer的index 和 getNewRecordBuffer的数组中index 是从0-1交替出现的，
 * 因此index++ 就是去获取下一个角标。拿到新的一个缓冲buffer
 *
 *
 *
 * @return 即将要录入PCM数据的buffer
 */
short *RecordBuffer::getNewRecordBuffer() {
    index++;
    if (index > 1) {
        index = 0;
    }
    return buffer[index];
}
