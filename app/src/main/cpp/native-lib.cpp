#include <jni.h>
#include <string>
#include "player/header/PlayerController.h"


JavaVM *pJavaVM;
PlayerController *pPlayerController = NULL;
JNICallback *pJniCallback = NULL;


// 重写 so 被加载时会调用的一个方法
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserved) {
    pJavaVM = javaVM;
    JNIEnv *env;
    if (javaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_4;
}


void release() {
    if (pPlayerController != NULL && pPlayerController->pPlayerStatus != NULL) {
        pPlayerController->pPlayerStatus->isExit = true;
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_tf_player_NativePlayer_nPlay(JNIEnv *env, jobject instance) {
    if (pPlayerController != NULL) {
        pPlayerController->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tf_player_NativePlayer_nStop(JNIEnv *env, jobject instance) {
    release();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tf_player_NativePlayer_nState(JNIEnv *env, jobject instance, jboolean isPlay) {
    if (pPlayerController != NULL) {
        pPlayerController->setPlayState(isPlay);
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_com_tf_player_NativePlayer_setSurface(JNIEnv *env, jobject instance, jobject surface) {
    if (pPlayerController != NULL) {
        pPlayerController->setSurface(surface);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tf_player_NativePlayer_nPrepareAsync(JNIEnv *env, jobject instance, jstring url_,
                                              jobject callback) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (pPlayerController != NULL) {
        release();
    }
    pJniCallback = new JNICallback(pJavaVM, env, callback);
    pPlayerController = new PlayerController(pJniCallback, url);
    pPlayerController->prepareAsync();
    env->ReleaseStringUTFChars(url_, url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_tf_player_NativePlayer_nRelease(JNIEnv *env, jobject instance) {
    if (pPlayerController != NULL) {
        release();
//        av_usleep(100000);
        delete pPlayerController;
        pPlayerController = NULL;
        LOGE("delete pPlayerController success");
    }
}