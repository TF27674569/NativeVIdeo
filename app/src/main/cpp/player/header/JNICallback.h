//
// Created by TIAN FENG on 2019/8/13.
//

#ifndef NATIVEVIDEO_JNICALLBACK_H
#define NATIVEVIDEO_JNICALLBACK_H

#include <jni.h>

#include "../header/ConstDefine.h"
#include <cwchar>

enum ThreadMode {
    THREAD_MAIN = 1, THREAD_CHILD
};

class JNICallback {
public:
    jmethodID prepareMid = NULL; // 准备完成了的函数id
    jmethodID errorMid = NULL; // 异常回调的函数id
    jmethodID progressMid = NULL; // 异常回调的函数id
    JavaVM *javaVM = NULL;// java虚拟机（需要处理子线程的）
    JNIEnv *jniEnv = NULL;// app环境
    jobject javaInstance = NULL;// 对应java层的NativeCall

public:
    JNICallback(JavaVM *javaVM, JNIEnv *jniEnv, jobject jPlayerObj);

    ~JNICallback();

public:
    void onError(ThreadMode threadMode, int code, char *msg);

    void onPrepared(ThreadMode mode);


    void onProgress(ThreadMode mode, int total, double current, double progress);
};


#endif //NATIVEVIDEO_JNICALLBACK_H
