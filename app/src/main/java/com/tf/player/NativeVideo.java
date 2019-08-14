package com.tf.player;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.tf.player.listener.OnErrorListener;
import com.tf.player.listener.OnPrepareListener;

/**
 * create by TIAN FENG on 2019/8/13
 */
public class NativeVideo extends SurfaceView implements OnPrepareListener, OnErrorListener {

    private NativePlayer mPlayer;

    public NativeVideo(Context context) {
        this(context, null);
    }

    public NativeVideo(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public NativeVideo(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        // 设置显示的像素格式
        SurfaceHolder holder = getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
        mPlayer = new NativePlayer();
        mPlayer.setOnPreparedListener(this);

        mPlayer.setOnErrorListener(this);
    }

    public void play(String uri) {
        stop();
        mPlayer.setDataSource(uri);
        mPlayer.prepareAsync();
    }

    /**
     * 停止方法，释放上一个视频的内存
     */
    public void stop() {
        mPlayer.stop();
    }

    @Override
    public void onPrepare() {
        mPlayer.setSurfaceView(this);
        mPlayer.play();
    }


    public void pause() {
        mPlayer.setPlayState(false);
    }

    public void resume() {
        mPlayer.setPlayState(true);
    }

    @Override
    public void onError(int code, String msg) {
        Log.e("TAG", "onError: " + msg);
    }

    public void release() {
        mPlayer.release();
    }
}
