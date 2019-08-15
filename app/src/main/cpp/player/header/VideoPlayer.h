//
// Created by TIAN FENG on 2019/8/13.
//

#ifndef NATIVEVIDEO_VIDEOPLAYER_H
#define NATIVEVIDEO_VIDEOPLAYER_H
extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
};

#include "BaseMedia.h"
#include "AudioPlayer.h"
#include <pthread.h>

class VideoPlayer : public BaseMedia {

public:
    SwsContext *pSwsContext = NULL;
    uint8_t *pFrameBuffer = NULL;
    int frameSize;
    AVFrame *pRgbaFrame = NULL;
    jobject surface = NULL;
    AudioPlayer *pAudio = NULL;
    jboolean isPause = false;
    float defaultDelayTime;


    // 一个线程去解码播放
    pthread_t playThreadT;
    pthread_mutex_t mutex;// 锁
    pthread_cond_t cond;// 信号

public:
    VideoPlayer(int streamIndex, JNICallback *pJniCall, PlayerStatus *pPlayerStatus,
                AudioPlayer *pAudio);

    ~VideoPlayer();

public:
    void play();

    void onDecodeStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    void release();

    void setSurface(jobject surface);

    /**
     * 视频同步音频，计算获取休眠的时间
     * @param pFrame 当前视频帧
     * @return 休眠时间（s）
     */
    double getFrameSleepTime(AVFrame *pFrame);

    void setPlayState(jboolean isPause);

};


#endif //NATIVEVIDEO_VIDEOPLAYER_H
