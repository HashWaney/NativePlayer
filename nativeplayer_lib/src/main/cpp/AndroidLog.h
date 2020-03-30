//
// Created by 王庆 on 2020-03-30.
//

#ifndef PLAYER_ANDROIDLOG_H
#define PLAYER_ANDROIDLOG_H
#include "android/log.h"
#define TAG "HASH"
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);



#endif //PLAYER_ANDROIDLOG_H
