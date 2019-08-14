//
// Created by TIAN FENG on 2019/8/13.
//

#ifndef NATIVEVIDEO_AUDIOPLAYER_H
#define NATIVEVIDEO_AUDIOPLAYER_H

#include "BaseMedia.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../header/ConstDefine.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};

class AudioPlayer : public BaseMedia {
public:
    SwrContext *pSwrContext = NULL;
    uint8_t *resampleOutBuffer = NULL;
    SLPlayItf pPlayItf = NULL;

public:
    AudioPlayer(int streamIndex, JNICallback *pJniCall, PlayerStatus *pPlayerStatus);

    ~AudioPlayer();

public:
    void play();

    void initCrateOpenSLES();

    int resampleAudio();

    void onDecodeStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    void release();

    void setPlayState(jboolean isPause);
};


#endif //NATIVEVIDEO_AUDIOPLAYER_H
