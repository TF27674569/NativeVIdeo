//
// Created by TIAN FENG on 2019/8/13.
//

#include "../header/PacketQueue.h"
#include "../header/ConstDefine.h"

PacketQueue::PacketQueue() {
    // 初始化队列
    pPacketQueue = new std::queue<AVPacket *>();
    // 初始化互斥锁
    pthread_mutex_init(&packetMutex, NULL);
    // 初始化信号
    pthread_cond_init(&packetCond, NULL);
}

PacketQueue::~PacketQueue() {
    if (pPacketQueue != NULL) {
        clear();
        delete (pPacketQueue);
        pPacketQueue = NULL;

        pthread_mutex_destroy(&packetMutex);
        pthread_cond_destroy(&packetCond);
    }

    LOGE("delete packet queue end  %p", this);
}


AVPacket *PacketQueue::pop() {
    AVPacket *pPacket;
    /**
     * 获取的时候先锁住
     */
    pthread_mutex_lock(&packetMutex);

    /**
     * 如果队列没有  让其等待  push 之后会发信号唤醒锁
     */
    while (pPacketQueue->empty()) {
        // 如果队列没有则让其等待
        pthread_cond_wait(&packetCond, &packetMutex);
    }
    pPacket = pPacketQueue->front();
    pPacketQueue->pop();
    pthread_mutex_unlock(&packetMutex);
    return pPacket;
}

void PacketQueue::clear() {
    if (pPacketQueue == NULL) return;
    AVPacket *pPacket;

    while (!pPacketQueue->empty()) {
        pPacket = pPacketQueue->front();
        pPacketQueue->pop();
//        av_packet_unref(pPacket);
        av_free(pPacket);
        pPacket = NULL;
    }

    LOGE("clear packet queue end  %p", this);
}

void PacketQueue::push(AVPacket *pPacket) {
    pthread_mutex_lock(&packetMutex);
    pPacketQueue->push(pPacket);
    pthread_cond_signal(&packetCond);
    pthread_mutex_unlock(&packetMutex);
}
