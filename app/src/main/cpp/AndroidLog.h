//
// Created by 王庆 on 2020-03-29.
//

#ifndef PTHREADDEMO_ANDROIDLOG_H
#define PTHREADDEMO_ANDROIDLOG_H

#include "android/log.h"
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"HASH",FORMAT,##__VA_ARGS__);

#endif //PTHREADDEMO_ANDROIDLOG_H
