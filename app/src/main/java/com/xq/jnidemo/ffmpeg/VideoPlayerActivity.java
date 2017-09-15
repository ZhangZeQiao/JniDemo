package com.xq.jnidemo.ffmpeg;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.Surface;
import android.view.View;

import com.xq.jnidemo.R;

import java.io.File;

/**
 * @author 小侨
 * @time 2017/9/15  14:11
 * @desc ${TODD}
 */

public class VideoPlayerActivity extends AppCompatActivity {

    private VideoView mVideoView;

    private void initView() {
        mVideoView = (VideoView) findViewById(R.id.video_view);
    }


    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_player_activity);

        initView();
    }

    /**
     * FFmpeg解码练习的调用代码
     */
    private void player() {
        String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
        String output = new File(Environment.getExternalStorageDirectory(), "output_1280x800_yuv420p.yuv").getAbsolutePath();
        VideoUtils.player(input, output);
    }

    public void onPlay(View view) {

        String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
        // surface 传入到 Native函数中，用于绘制
        Surface surface = mVideoView.getHolder().getSurface();
        VideoUtils.render(input, surface);

        /*new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
                    // surface 传入到 Native函数中，用于绘制
                    Surface surface = mVideoView.getHolder().getSurface();
                    VideoUtils.render(input, surface);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();*/
    }
}
