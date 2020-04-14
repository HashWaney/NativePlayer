/**
 * Created by Hash on 2020-04-13.
 */


#ifndef AUDIOPLAYER_AUDIOLOG_H
#define AUDIOPLAYER_AUDIOLOG_H

#include "android/log.h"

#define TAG "BlackJ"
#define ON  true

#define LOG_D(FORMAT, ...) __android_log_print(ANDROID_LOG_DEBUG,TAG,FORMAT,##__VA_ARGS__);
#define LOG_E(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,TAG,FORMAT,##__VA_ARGS__);


#endif //AUDIOPLAYER_AUDIOLOG_H
