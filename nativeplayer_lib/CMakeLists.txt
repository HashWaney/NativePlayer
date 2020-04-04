cmake_minimum_required(VERSION 3.4.1)

#添加ffmpeg的头文件
include_directories(src/main/cpp/include)


#工程顶层目录 PROJECT_SOURCE_DIR 也就是nativeplay_lib这个工程的当前目录
#CMAKE_ANDROID_API
#set(CMAKE_ANDROID_ARCH_ABI ${ANDROID_ABI})


add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavcodec.so)


add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavdevice.so)


add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavfilter.so)



add_library(avformat SHARED IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavformat.so)


add_library(avutil SHARED IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libavutil.so)


add_library(swresample SHARED IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libswresample.so)


add_library(swscale SHARED IMPORTED)

set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}/libswscale.so)



add_library(
        native-lib

        SHARED

        src/main/cpp/native-lib.cpp
        src/main/cpp/SimpleAvCodec.cpp
        src/main/cpp/CallJavaBridge.cpp
        src/main/cpp/AVPacketQueue.cpp
        src/main/cpp/AVPlayStatus.cpp
)

find_library( # Sets the name of the path variable.
        log-lib


        log)

target_link_libraries( # Specifies the target library.
        native-lib

        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        OpenSLES

        ${log-lib})