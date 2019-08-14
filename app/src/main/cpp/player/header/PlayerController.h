//
// Created by TIAN FENG on 2019/8/13.
//

#ifndef NATIVEVIDEO_PLAYERCONTROLLER_H
#define NATIVEVIDEO_PLAYERCONTROLLER_H

extern "C" {
#include "../../include/libavformat/avformat.h"
};

#include "JNICallback.h"
#include "PlayerStatus.h"
#include "AudioPlayer.h"
#include "VideoPlayer.h"
#include <jni.h>

class PlayerController {
public:
    AVFormatContext *pFormatContext = NULL;
    char *url = NULL;
    JNICallback *pJniCall = NULL;
    PlayerStatus *pPlayerStatus = NULL;
    AudioPlayer *pAudioPlayer = NULL;
    VideoPlayer *pVideoPlayer = NULL;

public:
    PlayerController(JNICallback *pJniCall, const char *url);

    ~PlayerController();

public:
    void play();

    void prepare();

    void prepareAsync();

    void prepare(ThreadMode threadMode);

    void onError(ThreadMode threadMode, int code, char *msg);

    void release();

    void setSurface(jobject surface);


    void setPlayState(jboolean isPause);
};


#endif //NATIVEVIDEO_PLAYERCONTROLLER_H
