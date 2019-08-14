package com.tf.player;

import android.view.Surface;
import android.view.SurfaceView;

import com.tf.player.listener.OnErrorListener;
import com.tf.player.listener.OnPrepareListener;

/**
 * create by TIAN FENG on 2019/8/13
 */
public class NativePlayer {

    static {
        System.loadLibrary("native-lib");
    }

    private NativeCall mNativeCall;
    private String mUrl;

    public NativePlayer() {
        mNativeCall = new NativeCall();
    }


    public void setOnPreparedListener(OnPrepareListener listener) {
        mNativeCall.setPrepareListener(listener);
    }

    public void setOnErrorListener(OnErrorListener listener) {
        mNativeCall.setOnErrorListener(listener);
    }


    public void setDataSource(String url) {
        mUrl = url;
    }

    public void prepareAsync() {
        checkNull(mUrl);
        nPrepareAsync(mUrl, mNativeCall);
    }

    public void stop() {
        nStop();
    }


    public void setPlayState(boolean isPlay) {
        checkNull(mUrl);
        nState(isPlay);
    }

    public void setSurfaceView(SurfaceView surface) {
        checkNull(mUrl);
        setSurface(surface.getHolder().getSurface());
    }

    public void play() {
        checkNull(mUrl);
        nPlay();
    }


    public void release() {
        nRelease();
    }

    private void checkNull(Object o) {
        if (null == o) {
            throw new NullPointerException("target is null!");
        }
    }

    private native void nPlay();

    private native void nStop();

    private native void nState(boolean isPlay);

    private native void nPrepareAsync(String url, NativeCall callback);

    private native void setSurface(Surface surface);

    private native void nRelease();

}
