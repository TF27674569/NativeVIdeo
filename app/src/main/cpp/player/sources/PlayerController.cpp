//
// Created by TIAN FENG on 2019/8/13.
//

#include "../header/PlayerController.h"

PlayerController::PlayerController(JNICallback *pJniCall, const char *url) {
    this->pJniCall = pJniCall;
    // 赋值一份 url ，因为怕外面方法结束销毁了 url
    this->url = (char *) malloc(strlen(url) + 1);
    memcpy(this->url, url, strlen(url) + 1);
    pPlayerStatus = new PlayerStatus();
    pPlayerStatus->isExit = false;
}

PlayerController::~PlayerController() {
    release();
}

void *threadReadPacket(void *context) {
    PlayerController *pFFmpeg = (PlayerController *) context;
    while (pFFmpeg->pPlayerStatus != NULL && !pFFmpeg->pPlayerStatus->isExit) {
        AVPacket *pPacket = av_packet_alloc();
        if (av_read_frame(pFFmpeg->pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == pFFmpeg->pAudioPlayer->streamIndex) {
                pFFmpeg->pAudioPlayer->pPacketQueue->push(pPacket);
            } else if (pPacket->stream_index == pFFmpeg->pVideoPlayer->streamIndex) {
                pFFmpeg->pVideoPlayer->pPacketQueue->push(pPacket);
            } else {
                av_packet_free(&pPacket);
            }
        } else {
            av_packet_free(&pPacket);
        }
    }
    return 0;
}

void PlayerController::play() {
    // 一个线程去读取 Packet
    pthread_t readPacketThreadT;
    pthread_create(&readPacketThreadT, NULL, threadReadPacket, this);
    pthread_detach(readPacketThreadT);

    if (pAudioPlayer != NULL) {
        pAudioPlayer->play();
    }

    if (pVideoPlayer != NULL) {
        pVideoPlayer->play();
    }
}

void PlayerController::prepare() {
    prepare(THREAD_MAIN);
}

void *threadPrepare(void *context) {
    PlayerController *controller = (PlayerController *) context;
    controller->prepare(THREAD_CHILD);
    return 0;
}

void PlayerController::prepareAsync() {
// 创建一个线程去播放，多线程编解码边播放
    pthread_t prepareThreadT;
    pthread_create(&prepareThreadT, NULL, threadPrepare, this);
    pthread_detach(prepareThreadT);
}

void PlayerController::prepare(ThreadMode threadMode) {
    av_register_all();
    avformat_network_init();
    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;

    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        LOGE("format open input error: %s,url:%s", av_err2str(formatOpenInputRes), url);
        onError(threadMode, formatOpenInputRes, av_err2str(formatOpenInputRes));
        return;
    }

    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error: %s", av_err2str(formatFindStreamInfoRes));
        onError(threadMode, formatFindStreamInfoRes,
                av_err2str(formatFindStreamInfoRes));
        return;
    }

    // 查找音频流的 index
    int audioStreamIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO, -1,
                                               -1, NULL, 0);
    if (audioStreamIndex < 0) {
        LOGE("format audio stream error.");
        onError(threadMode, FIND_STREAM_ERROR_CODE, "find audio stream error");
        return;
    }

    pAudioPlayer = new AudioPlayer(audioStreamIndex, pJniCall, pPlayerStatus);
    pAudioPlayer->decodeStream(threadMode, pFormatContext);

    // 查找视频流的 index
    int videoStreamIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_VIDEO, -1,
                                               -1, NULL, 0);
    // 如果没有视频就直接播放音频
    /* if (videoStreamIndex < 0) {
        return;
    }*/
    if (videoStreamIndex < 0) {
        LOGE("format video stream error.");
        onError(threadMode, FIND_STREAM_ERROR_CODE, "find video stream error");
        return;
    }
    // 不是我的事我不干，但是大家也不要想得过于复杂
    pVideoPlayer = new VideoPlayer(videoStreamIndex, pJniCall, pPlayerStatus, pAudioPlayer);
    pVideoPlayer->decodeStream(threadMode, pFormatContext);

    // 回调到 Java 告诉他准备好了
    pJniCall->onPrepared(threadMode);
}

void PlayerController::onError(ThreadMode threadMode, int code, char *msg) {
// 释放资源
    release();
    // 回调给 java 层调用
    pJniCall->onError(threadMode, code, msg);
}

void PlayerController::release() {
    if (pFormatContext != NULL) {
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }

    avformat_network_deinit();

    if (url != NULL) {
        free(url);
        url = NULL;
    }

    if (pPlayerStatus != NULL) {
        delete (pPlayerStatus);
        pPlayerStatus = NULL;
    }

    if (pAudioPlayer != NULL) {
        delete (pAudioPlayer);
        pAudioPlayer = NULL;
    }

    if (pVideoPlayer != NULL) {
        delete (pVideoPlayer);
        pVideoPlayer = NULL;
    }
}

void PlayerController::setSurface(jobject surface) {
    if (pVideoPlayer != NULL) {
        pVideoPlayer->setSurface(surface);
    }
}

void PlayerController::setPlayState(jboolean isPause) {
    if (pAudioPlayer != NULL) {
        pAudioPlayer->setPlayState(isPause);
    }

    if (pVideoPlayer != NULL) {
        pVideoPlayer->setPlayState(isPause);
    }
}
