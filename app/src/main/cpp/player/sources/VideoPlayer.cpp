//
// Created by TIAN FENG on 2019/8/13.
//

#include "../header/VideoPlayer.h"

VideoPlayer::VideoPlayer(int streamIndex, JNICallback *pJniCall, PlayerStatus *pPlayerStatus,
                         AudioPlayer *pAudio)
        : BaseMedia(streamIndex, pJniCall, pPlayerStatus) {
    this->pAudio = pAudio;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

VideoPlayer::~VideoPlayer() {
    release();
}

void *threadVideoPlay(void *context) {
    VideoPlayer *pVideo = (VideoPlayer *) context;
    // 获取当前线程的 JNIEnv， 通过 JavaVM
    JNIEnv *env;
    if (pVideo->pJniCall->javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
        LOGE("get child thread jniEnv error!");
        return 0;
    }
    // 获取窗体
    ANativeWindow *pNativeWindow = ANativeWindow_fromSurface(env, pVideo->surface);
    pVideo->pJniCall->javaVM->DetachCurrentThread();

    // 设置缓存区的数据
    ANativeWindow_setBuffersGeometry(pNativeWindow, pVideo->pCodecContext->width,
                                     pVideo->pCodecContext->height, WINDOW_FORMAT_RGBA_8888);
    // Window 缓冲区的 Buffer
    ANativeWindow_Buffer outBuffer;

    AVPacket *pPacket = NULL;
    AVFrame *pFrame = av_frame_alloc();

    while (pVideo->pPlayerStatus != NULL && !pVideo->pPlayerStatus->isExit) {
        pthread_mutex_lock(&(pVideo->mutex));

        // 视频被暂停  让其等待
        if (pVideo->isPause) {
            pthread_cond_wait(&(pVideo->cond), &(pVideo->mutex));
        }

        pPacket = pVideo->pPacketQueue->pop();
        // Packet 包，压缩的数据，解码成 pcm 数据
        int codecSendPacketRes = avcodec_send_packet(pVideo->pCodecContext, pPacket);
        if (codecSendPacketRes == 0) {
            int codecReceiveFrameRes = avcodec_receive_frame(pVideo->pCodecContext, pFrame);
            if (codecReceiveFrameRes == 0) {
                sws_scale(pVideo->pSwsContext, (const uint8_t *const *) pFrame->data,
                          pFrame->linesize,
                          0, pVideo->pCodecContext->height, pVideo->pRgbaFrame->data,
                          pVideo->pRgbaFrame->linesize);
                // 在播放之前判断一下需要休眠多久
                double frameSleepTime = pVideo->getFrameSleepTime(pFrame);
                av_usleep(frameSleepTime * 1000000);
                // 回调进度
                double progress = (1000000 * (pVideo->currentTime) / (pVideo->totalDuration));
                pVideo->pJniCall->onProgress(THREAD_CHILD, pVideo->totalDuration,
                                             (pVideo->currentTime) * 1000000.0, progress);
                // 把数据推到缓冲区
                ANativeWindow_lock(pNativeWindow, &outBuffer, NULL);
                memcpy(outBuffer.bits, pVideo->pFrameBuffer, pVideo->frameSize);
                ANativeWindow_unlockAndPost(pNativeWindow);
            }
        }
        // 解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
        pthread_mutex_unlock(&(pVideo->mutex));
    }
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    pthread_cond_signal(&(pVideo->cond));
    return 0;
}

void VideoPlayer::play() {
    pthread_create(&playThreadT, NULL, threadVideoPlay, this);
    pthread_detach(playThreadT);
}

void VideoPlayer::onDecodeStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {
    pSwsContext = sws_getContext(pCodecContext->width, pCodecContext->height,
                                 pCodecContext->pix_fmt, pCodecContext->width,
                                 pCodecContext->height,
                                 AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
    pRgbaFrame = av_frame_alloc();
    frameSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecContext->width,
                                         pCodecContext->height, 1);
    pFrameBuffer = (uint8_t *) malloc(frameSize);
    av_image_fill_arrays(pRgbaFrame->data, pRgbaFrame->linesize, pFrameBuffer, AV_PIX_FMT_RGBA,
                         pCodecContext->width, pCodecContext->height, 1);
    int num = pFormatContext->streams[streamIndex]->avg_frame_rate.num;
    int den = pFormatContext->streams[streamIndex]->avg_frame_rate.den;
    if (den != 0 && num != 0) {
        defaultDelayTime = 1.0f * den / num;
    }
}

void VideoPlayer::release() {
    // 线程解码 推suface数据时 可能出现正在睡眠,此时销毁队列异常
    av_usleep(100000);
    BaseMedia::release();
    if (pSwsContext != NULL) {
        sws_freeContext(pSwsContext);
        pSwsContext = NULL;
    }

    if (pFrameBuffer != NULL) {
        free(pFrameBuffer);
        pFrameBuffer = NULL;
    }

    if (pRgbaFrame != NULL) {
        av_frame_free(&pRgbaFrame);
        pRgbaFrame = NULL;
    }

    if (pJniCall != NULL) {
        pJniCall->jniEnv->DeleteGlobalRef(surface);
        surface = NULL;
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void VideoPlayer::setSurface(jobject surface) {
    this->surface = pJniCall->jniEnv->NewGlobalRef(surface);
}

double VideoPlayer::getFrameSleepTime(AVFrame *pFrame) {
    double times = av_frame_get_best_effort_timestamp(pFrame) * av_q2d(timeBase);// s
    // 相差多少秒
    double diffTime = pAudio->currentTime - times;
    currentTime = times;
    return -diffTime;
}

void VideoPlayer::setPlayState(jboolean isPause) {
    this->isPause = isPause;
    if (!isPause) {
        pthread_cond_signal(&cond);
    }
}
