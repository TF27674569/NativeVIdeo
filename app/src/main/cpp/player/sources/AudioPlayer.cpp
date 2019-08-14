//
// Created by TIAN FENG on 2019/8/13.
//

#include "../header/AudioPlayer.h"

AudioPlayer::AudioPlayer(int streamIndex, JNICallback *pJniCall, PlayerStatus *pPlayerStatus)
        : BaseMedia(streamIndex, pJniCall, pPlayerStatus) {}

AudioPlayer::~AudioPlayer() {
    release();
}


void *threadAudioPlay(void *context) {
    AudioPlayer *pAudio = (AudioPlayer *) context;
    pAudio->initCrateOpenSLES();
    return 0;
}

void AudioPlayer::play() {
    // 一个线程去解码播放
    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadAudioPlay, this);
    pthread_detach(playThreadT);
}

// openSLES 的回调函数
void playerCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    AudioPlayer *pAudio = (AudioPlayer *) pContext;
    // 去拿队列的avpacket  放入openSLES中进行 播放
    int dataSize = pAudio->resampleAudio();
    // 这里为什么报错，留在后面再去解决
    (*caller)->Enqueue(caller, pAudio->resampleOutBuffer, dataSize);
}


void AudioPlayer::initCrateOpenSLES() {
    //1 创建引擎接口对象
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    // realize the engine
    (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    // get the engine interface, which is needed in order to create other objects
    (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    // 2 设置混音器
    static SLObjectItf outputMixObject = NULL;
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                     &outputMixEnvironmentalReverb);
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,
                                                                      &reverbSettings);
    // 3 创建播放器
    SLObjectItf pPlayer = NULL;
    SLDataLocator_AndroidSimpleBufferQueue simpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&simpleBufferQueue, &formatPcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    SLInterfaceID interfaceIds[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE};
    SLboolean interfaceRequired[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine, &pPlayer, &audioSrc, &audioSnk, 3,
                                       interfaceIds, interfaceRequired);
    (*pPlayer)->Realize(pPlayer, SL_BOOLEAN_FALSE);
    (*pPlayer)->GetInterface(pPlayer, SL_IID_PLAY, &pPlayItf);
    // 4 设置缓存队列和回调函数
    SLAndroidSimpleBufferQueueItf playerBufferQueue;
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    // 每次回调 this 会被带给 playerCallback 里面的 context
    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, playerCallback, this);
    // 5 设置播放状态
    (*pPlayItf)->SetPlayState(pPlayItf, SL_PLAYSTATE_PLAYING);
    // 6 调用回调函数
    playerCallback(playerBufferQueue, this);
}

int AudioPlayer::resampleAudio() {

    int dataSize = 0;
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = av_frame_alloc();

    while (pPlayerStatus != NULL && !pPlayerStatus->isExit) {
        pPacket = pPacketQueue->pop();
        // Packet 解码成 pcm 数据
        int codecSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
        if (codecSendPacketRes == 0) {
            int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext, pFrame);
            if (codecReceiveFrameRes == 0) {
                // AVPacket -> AVFrame
                // 调用重采样的方法，返回值是返回重采样的个数，也就是 pFrame->nb_samples
                dataSize = swr_convert(pSwrContext, &resampleOutBuffer, pFrame->nb_samples,
                                       (const uint8_t **) pFrame->data, pFrame->nb_samples);
                dataSize = dataSize * 2 * 2;

                // 设置当前的时间，回调进度给 Java ，视频同步音频
                double times = av_frame_get_best_effort_timestamp(pFrame) * av_q2d(timeBase);// s
                if (times > currentTime) {
                    currentTime = times;
                }
                break;
            }
        }
        // 解引用
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    return dataSize;
}

void AudioPlayer::onDecodeStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {
    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;// 立体声
    enum AVSampleFormat out_sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    int out_sample_rate = AUDIO_SAMPLE_RATE;
    int64_t in_ch_layout = pCodecContext->channel_layout;
    enum AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
    int in_sample_rate = pCodecContext->sample_rate;
    pSwrContext = swr_alloc_set_opts(NULL, out_ch_layout, out_sample_fmt,
                                     out_sample_rate, //输出声道，格式，采样率
                                     in_ch_layout, in_sample_fmt, in_sample_rate,//输入声道，格式，采样率
                                     0, NULL);
    if (pSwrContext == NULL) {
        // 提示错误
        onError(threadMode, SWR_ALLOC_SET_OPTS_ERROR_CODE, "swr alloc set opts error");
        return;
    }
    int swrInitRes = swr_init(pSwrContext);
    if (swrInitRes < 0) {
        onError(threadMode, SWR_CONTEXT_INIT_ERROR_CODE, "swr context swr init error");
        return;
    }

    // buffer大小
    resampleOutBuffer = (uint8_t *) malloc(pCodecContext->frame_size * 2 * 2);
}

void AudioPlayer::release() {
    BaseMedia::release();
    if (resampleOutBuffer) {
        free(resampleOutBuffer);
        resampleOutBuffer = NULL;
    }

    if (pSwrContext != NULL) {
        swr_free(&pSwrContext);
        free(pSwrContext);
        pSwrContext = NULL;
    }
}

void AudioPlayer::setPlayState(jboolean isPause) {
    if (pPlayItf != NULL) {
        (*pPlayItf)->SetPlayState(pPlayItf, isPause ? SL_PLAYSTATE_PAUSED : SL_PLAYSTATE_PLAYING);
    }
}
