package com.tf.nativevideo;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import com.tf.player.NativeVideo;

import java.io.File;


public class MainActivity extends AppCompatActivity {

    private String videoPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/input.mp4";

    private NativeVideo video;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        video = findViewById(R.id.videoView);
    }

    public void play(View view) {
        Log.e("TAG", "file is exists " + new File(videoPath).exists());

        video.play(videoPath);
    }

    public void stop(View view) {
        video.stop();
    }

    private boolean isPause = false;

    public void pause(View view) {
        Button button = (Button) view;
        if (isPause) {
            video.pause();
            button.setText("pause");
        } else {
            video.resume();
            button.setText("resume");
        }
        isPause = !isPause;
    }

    public void release(View view) {
        video.release();
    }
}
