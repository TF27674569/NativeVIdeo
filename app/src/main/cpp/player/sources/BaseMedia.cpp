//
// Created by TIAN FENG on 2019/8/13.
//

#include "../header/BaseMedia.h"
#include "../header/ConstDefine.h"

BaseMedia::BaseMedia(int streamIndex, JNICallback *pJniCall, PlayerStatus *pPlayerStatus) {
    this->streamIndex = streamIndex;
    this->pJniCall = pJniCall;
    this->pPlayerStatus = pPlayerStatus;
    pPacketQueue = new PacketQueue();
}

BaseMedia::~BaseMedia() {
    release();
}

void BaseMedia::decodeStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {
    // 查找解码器
    AVCodecParameters *pCodecParameters = pFormatContext->streams[streamIndex]->codecpar;
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGE("codec find audio decoder error");
        onError(threadMode, CODEC_FIND_DECODER_ERROR_CODE, "codec find audio decoder error");
        return;
    }

    // 打开解码器
    pCodecContext = avcodec_alloc_context3(pCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        onError(threadMode, CODEC_ALLOC_CONTEXT_ERROR_CODE, "codec alloc context error");
        return;
    }

    int codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext,
                                                                    pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error: %s", av_err2str(codecParametersToContextRes));
        onError(threadMode, codecParametersToContextRes,
                av_err2str(codecParametersToContextRes));
        return;
    }

    int codecOpenRes = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error: %s", av_err2str(codecOpenRes));
        onError(threadMode, codecOpenRes, av_err2str(codecOpenRes));
        return;
    }

    this->totalDuration = pFormatContext->duration;
    timeBase = pFormatContext->streams[streamIndex]->time_base;

    ///  后续的解码进 队列需要 子类自己实现
    this->onDecodeStream(threadMode, pFormatContext);

}

void BaseMedia::release() {
    if (pPacketQueue != NULL) {
        delete (pPacketQueue);
        pPacketQueue = NULL;
    }

    if (pCodecContext != NULL) {
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }
}

void BaseMedia::onError(ThreadMode threadMode, int code, char *msg) {
// 释放资源
    release();
    // 回调给 java 层调用
    pJniCall->onError(threadMode, code, msg);
}
