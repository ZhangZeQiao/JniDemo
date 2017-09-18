package com.xq.jnidemo.ffmpeg;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Surface;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import com.xq.jnidemo.R;

import java.io.File;

/**
 * @author 小侨
 * @time 2017/9/15  14:11
 * @desc ${TODD}
 */

public class VideoPlayerActivity extends AppCompatActivity {

    private VideoView mVideoView;
    private Spinner mSpVideo;


    private void initView() {
        mVideoView = (VideoView) findViewById(R.id.video_view);
        mSpVideo = (Spinner) findViewById(R.id.sp_video);
        // 多种格式的视频列表
        String[] videoArray = getResources().getStringArray(R.array.video_array);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_list_item_1, android.R.id.text1, videoArray);
        mSpVideo.setAdapter(adapter);
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

        String input = new File(Environment.getExternalStorageDirectory(), mSpVideo.getSelectedItem().toString()).getAbsolutePath();
        Log.v("xq",input);
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
