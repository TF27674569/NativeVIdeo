package com.tf.player;

import android.os.Handler;
import android.util.Log;

import com.tf.player.listener.OnErrorListener;
import com.tf.player.listener.OnPrepareListener;

/**
 * create by TIAN FENG on 2019/8/13
 */


class NativeCall {

    private Handler mHandler = new Handler();
    private OnPrepareListener mPrepareListener;
    private OnErrorListener mErrorListener;

    void setPrepareListener(OnPrepareListener listener) {
        mPrepareListener = listener;
    }

    void setOnErrorListener(OnErrorListener listener) {
        mErrorListener = listener;
    }

    public void onPrepare() {
        if (mPrepareListener != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    mPrepareListener.onPrepare();
                }
            });
        }
    }

    public void onError(final int code, final String msg) {
        if (mErrorListener != null) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    mErrorListener.onError(code, msg);
                }
            });
        }
    }

    public void onProgress(int total, double current, double progress) {
        Log.e("TAG", "onProgress: total:" + total + "  current:" + current + "  progress:" + progress + " thread:" + Thread.currentThread().getName());
    }
}
