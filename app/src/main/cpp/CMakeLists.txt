# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(
        native-lib
        SHARED
        player/sources/JNICallback.cpp
        player/sources/PlayerController.cpp
        player/sources/BaseMedia.cpp
        player/sources/PacketQueue.cpp
        player/sources/AudioPlayer.cpp
        player/sources/VideoPlayer.cpp
        player/sources/PlayerStatus.cpp
        player/header/ConstDefine.h
        native-lib.cpp)

find_library(
        log-lib
        log)


#引入头文件
include_directories(${CMAKE_SOURCE_DIR}/include)


# 编解码库
add_library(avcodec-57
        SHARED
        IMPORTED)
set_target_properties(avcodec-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavcodec-57.so)

#输出输入设备的支持
add_library(avdevice-57
        SHARED
        IMPORTED)
set_target_properties(avdevice-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavdevice-57.so)

#滤镜
add_library(avfilter-6
        SHARED
        IMPORTED)
set_target_properties(avfilter-6
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavfilter-6.so)


#数据封装格式的库
add_library(avformat-57
        SHARED
        IMPORTED)
set_target_properties(avformat-57
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavformat-57.so)


# 工具包
add_library(avutil-55
        SHARED
        IMPORTED)
set_target_properties(avutil-55
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavutil-55.so)

#后期加工处理滤镜没眼
add_library(postproc-54
        SHARED
        IMPORTED)
set_target_properties(postproc-54
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libpostproc-54.so)

#重采样
add_library(swresample-2
        SHARED
        IMPORTED)
set_target_properties(swresample-2
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libswresample-2.so)


#像素格式转换 缩放
add_library(swscale-4
        SHARED
        IMPORTED)
set_target_properties(swscale-4
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libswscale-4.so)


#引入第三方so
link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/)


target_link_libraries(
        native-lib
        #链接第三方库
        avcodec-57
        avdevice-57
        avfilter-6
        avformat-57
        avutil-55
        postproc-54
        swresample-2
        swscale-4
        android

        OpenSLES
        ${log-lib})