//
// Created by TIAN FENG on 2019/8/13.
//

#include "../header/JNICallback.h"

JNICallback::JNICallback(JavaVM *javaVM, JNIEnv *jniEnv, jobject jCallback) {
    this->javaVM = javaVM;
    this->jniEnv = jniEnv;
    this->javaInstance = jniEnv->NewGlobalRef(jCallback);

    jclass jPlayerClass = jniEnv->GetObjectClass(javaInstance);
    errorMid = jniEnv->GetMethodID(jPlayerClass, "onError", "(ILjava/lang/String;)V");
    prepareMid = jniEnv->GetMethodID(jPlayerClass, "onPrepare", "()V");
    progressMid = jniEnv->GetMethodID(jPlayerClass, "onProgress", "(IDD)V");
}

JNICallback::~JNICallback() {
    if (javaInstance != NULL) {
        jniEnv->DeleteGlobalRef(javaInstance);
        javaInstance = NULL;
    }
}

void JNICallback::onError(ThreadMode threadMode, int code, char *msg) {
    if (threadMode == THREAD_MAIN) {
        jstring jMsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(javaInstance, errorMid, code, jMsg);
        jniEnv->DeleteLocalRef(jMsg);
    } else if (threadMode == THREAD_CHILD) {
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        jstring jMsg = env->NewStringUTF(msg);
        env->CallVoidMethod(javaInstance, errorMid, code, jMsg);
        env->DeleteLocalRef(jMsg);

        javaVM->DetachCurrentThread();
    }
}

void JNICallback::onPrepared(ThreadMode threadMode) {
    if (threadMode == THREAD_MAIN) {
        jniEnv->CallVoidMethod(javaInstance, prepareMid);
    } else if (threadMode == THREAD_CHILD) {
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }
        env->CallVoidMethod(javaInstance, prepareMid);
        javaVM->DetachCurrentThread();
    }
}

void JNICallback::onProgress(ThreadMode threadMode, int total, double current, double progress) {
    if (threadMode == THREAD_MAIN) {
        jniEnv->CallVoidMethod(javaInstance, progressMid, total, current, progress);
    } else if (threadMode == THREAD_CHILD) {
        // 获取当前线程的 JNIEnv， 通过 JavaVM
        JNIEnv *env;
        if (javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
            LOGE("get child thread jniEnv error!");
            return;
        }

        env->CallVoidMethod(javaInstance, progressMid, total / 1000, current / 1000, progress);
        javaVM->DetachCurrentThread();
    }
}
