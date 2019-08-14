//
// Created by TIAN FENG on 2019/8/13.
// 音频与视频的解码 AVPacket 队列
//

#ifndef NATIVEVIDEO_PACKETQUEUE_H
#define NATIVEVIDEO_PACKETQUEUE_H

#include <queue>
#include <pthread.h>

extern "C" {
#include <libavcodec/avcodec.h>
};

class PacketQueue {
public:
    std::queue<AVPacket *> *pPacketQueue;// 队列
    pthread_mutex_t packetMutex;// 锁
    pthread_cond_t packetCond;// 信号

public:
    PacketQueue();

    ~PacketQueue();

public:
    void push(AVPacket *pPacket);

    AVPacket *pop();


    void clear();

};


#endif //NATIVEVIDEO_PACKETQUEUE_H
