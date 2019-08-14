//
// Created by TIAN FENG on 2019/8/13.
// 解析视频与音频流 共同属性的基类
//

#ifndef NATIVEVIDEO_BASEMEDIA_H
#define NATIVEVIDEO_BASEMEDIA_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};


#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "JNICallback.h"
#include "PacketQueue.h"
#include "PlayerStatus.h"

class BaseMedia {
public:
    int streamIndex = -1; // 流角标
    AVCodecContext *pCodecContext = NULL;// 解码器上下文
    JNICallback *pJniCall;
    PacketQueue *pPacketQueue = NULL;
    long totalDuration = 0;// 总时长
    double currentTime = 0;//当前时间
    double lastUpdateTime = 0;// 最后一次回调时间
    AVRational timeBase;// 时间基
    PlayerStatus *pPlayerStatus;// 播放状态

public:
    BaseMedia(int streamIndex, JNICallback *pJniCall, PlayerStatus *pPlayerStatus);

    ~BaseMedia();

public:
public:

    // 纯虚函数  子类实现
    virtual void play() = 0;

    // 解析流
    void decodeStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    // 子类解析avpacket
    virtual void onDecodeStream(ThreadMode threadMode, AVFormatContext *pFormatContext) = 0;

    virtual void release();

    void onError(ThreadMode threadMode, int code, char *msg);

    virtual void setPlayState(jboolean isPause) = 0;

};


#endif //NATIVEVIDEO_BASEMEDIA_H
